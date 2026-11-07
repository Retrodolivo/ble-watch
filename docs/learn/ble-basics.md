# BLE basics (Step 3)

## Roles
- The watch is a **peripheral** (it advertises and waits). The phone is a **central** (it scans and connects).
- In GATT terms the watch is the **server** (it holds the data), the phone is the **client** (it reads/writes).

## Advertising (app/src/ble.c)
- `ad[]`: the advertising packet (max 31 bytes): flags + the 16-bit UUID of the Current Time Service (0x1805).
- `sd[]`: the scan response, sent only when the phone asks: the device name "ble-watch".
- `BT_LE_ADV_CONN_FAST_1`: connectable, fast interval (30-60 ms) so the phone finds it quickly.
- In this Zephyr version advertising stops on connect and does **not** restart by itself.
  We restart it in the `recycled` callback (the connection object is free again), from a work item.

## GATT: services and characteristics
- A **service** groups **characteristics**. Each has a UUID; 16-bit UUIDs are standard (Bluetooth SIG).
- Current Time Service 0x1805 (Zephyr: `CONFIG_BT_CTS`, `subsys/bluetooth/services/cts.c`):
  - Current Time 0x2A2B: read, write, notify. 10 bytes.
  - Local Time Information 0x2A0F: time zone + DST (we store it, but don't use it).
- Zephyr defines the attribute table; we only give callbacks (`struct bt_cts_cb`).

## Current Time format (10 bytes, little endian)
| Byte | Field | Example (2026-11-07 19:30:00, Saturday) |
|------|-------|------|
| 0-1 | year | `EA 07` (2026 = 0x07EA, low byte first) |
| 2 | month (1-12) | `0B` |
| 3 | day | `07` |
| 4 | hours | `13` |
| 5 | minutes | `1E` |
| 6 | seconds | `00` |
| 7 | day of week (1=Mon .. 7=Sun) | `06` |
| 8 | fractions of a second (1/256) | `00` |
| 9 | adjust reason (bit 0 = manual) | `01` |

Full value: `EA070B07131E00060001`.

## Threads and locking
- GATT write callbacks run in the Bluetooth RX thread, not in main.
- `clock_set()` and `clock_now()` share two variables, so they use a `k_spinlock`.
- `clock_set()` gives a semaphore, so the main loop wakes up and logs the new time at once.

## Spinlock vs mutex vs semaphore
- **Spinlock**: protects data. Never sleeps. On a single core it just disables interrupts.
  Use for a few instructions; allowed in an ISR. Our clock variables use it.
- **Mutex**: protects data too, but a waiting thread *sleeps*. Use when the protected code is long
  or can block (e.g. an SPI transfer). Not allowed in an ISR. Has priority inheritance.
- **Semaphore**: a *signal* ("something happened"), not a lock here. Our main loop sleeps on it
  with a timeout; `clock_set()` gives it to wake the loop early.
- Why the lock at all: `base_ms` and `base_uptime_ms` are 64-bit, and a 32-bit Cortex-M4 reads
  each in two steps. Without the lock, main could mix old and new values and show a wrong time.

## Controller vs host
- **Host** (Zephyr): GATT, ATT, GAP, the API we call.
- **Controller** (link layer + radio): Nordic's **SoftDevice Controller** in NCS (`CONFIG_BT_LL_SOFTDEVICE=y`).
