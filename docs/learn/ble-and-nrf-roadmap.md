# BLE and nRF peripherals: a learning roadmap

Goal: open the "black box" just enough for this project. Nothing more.
Learn in layers. Go to the next layer only when the project needs it.

## Layer 0: the mental model (read this first)

### The software stack, top to bottom
```
our code (app/src/ble.c, clock.c, main.c)
  Zephyr APIs (bt_le_adv_start, display_write, k_uptime_get, ...)
  Zephyr drivers / BT host
  nrfx  (Nordic's thin C layer over the registers)
  hardware peripherals (registers at fixed memory addresses)
```
Each layer only hides the one below. If something is a mystery, go one layer down.

### What an nRF peripheral is
A peripheral is a hardware block next to the CPU (RTC, SPIM, UARTE, GPIO, RADIO, ...).
The CPU talks to it through **registers**: memory addresses. Writing to one does something.
Nordic peripherals all follow the same pattern, so learning one teaches you all:
- **TASKS**: write `1` to start an action (e.g. `TASKS_START`).
- **EVENTS**: the hardware sets these when something happened (e.g. `EVENTS_END`).
- **INTENSET**: choose which events raise an interrupt to the CPU.
- **SHORTS**: link an event to a task *inside the same peripheral* (e.g. END -> STOP).
- **PPI**: link an event of one peripheral to a task of *another*, with no CPU at all.
- **EasyDMA**: peripherals like SPIM/UARTE read and write RAM by themselves.

### The peripherals this project uses
| Peripheral | What it does | Who uses it here |
|---|---|---|
| CLOCK | starts the crystals: 32 MHz (HFXO) and 32.768 kHz (LFXO) | Zephyr and the BLE controller |
| RTC1 | low-power counter on the 32.768 kHz clock | Zephyr kernel timer, so `k_uptime_get()` and our clock |
| GPIO / GPIOTE | pins, and pin interrupts | LEDs, display DC/RST/BUSY |
| SPIM | SPI master with EasyDMA | e-ink display (`arduino_spi`) |
| UARTE | serial port with EasyDMA | logs on the DK's USB serial |
| RADIO (+ RTC0, TIMER0, some PPI channels, crypto) | 2.4 GHz radio | **owned by the SoftDevice Controller**, not by us |
| NVMC (flash) | write and erase internal flash | MCUboot and updates (Step 4) |
| POWER | sleep modes, supply | idle current (Step 2) |

### BLE in one picture
```
Host  (Zephyr, runs on our CPU)           GAP   - who advertises, who connects
                                          GATT  - services and characteristics (our CTS)
                                          ATT   - read/write/notify on attributes
                                          SMP   - pairing and keys
                                          L2CAP - splits data into channels
---- HCI (the line between host and controller) ----
Controller (Nordic SoftDevice Controller) Link Layer - advertising, connection events, channel hopping
                                          PHY - the radio: 2.4 GHz, 40 channels (3 for advertising)
```
- Advertising: the watch shouts a short packet (max 31 bytes) on 3 channels, every N ms.
- Connection: after the phone connects, both wake up together every *connection interval*
  (e.g. 30 ms), exchange packets, and sleep again. That is why BLE is "low energy".
- GATT is the only part most apps touch: a table of attributes the phone reads and writes.

## Layer 1: one course + one book (the core)
1. **Nordic DevAcademy: "Bluetooth Low Energy Fundamentals"** (free, academy.nordicsemi.com).
   Uses nRF Connect SDK and the nRF52840 DK, with exercises. Best fit for this project.
   Lessons 1-4 (intro, advertising, connections, GATT) match Step 3. Lesson 5 (security) is optional.
   The "nRF Connect SDK Fundamentals" course is good too, for drivers, devicetree and threads.
   (Nordic's download server blocks our IP. If the academy is blocked too, use the book.)
2. **Book: "Getting Started with Bluetooth Low Energy"** (Townsend, Cufí, Akiba, Davidson; O'Reilly).
   About 180 pages. Old (BLE 4.1), but the basics have not changed. Read chapters 1-4 (intro,
   protocol basics, GAP, GATT). Skip the platform chapters at the end.

## Layer 2: reference, only when you need it (don't read cover to cover)
- **nRF52840 Product Specification** (docs.nordicsemi.com). Read one peripheral chapter at a time,
  when we use it: RTC (Step 2), SPIM (display), POWER and CLOCK (Step 2 current), NVMC (Step 4).
  Start with the chapter "Peripheral interface" - it explains TASKS/EVENTS/SHORTS for all of them.
- **Zephyr docs, Bluetooth section**: "Bluetooth Stack Architecture" and the GATT API.
- **Bluetooth SIG: "The Bluetooth Low Energy Primer"** (free PDF): a longer, deeper overview.
- **Bluetooth Core Specification**: never read it; only search it for one exact detail.

## Hands-on: look inside the black box on our own DK
1. In nRF Connect (Android), scan, open "ble-watch", and look at the raw advertising bytes.
   Find our flags and the `0x1805` UUID from `ad[]` in `app/src/ble.c`.
2. Connect and look at the attribute table: services, characteristics, their properties.
3. In the debugger (see `debugging.md`), open the peripheral view, find **RTC1** and watch
   `COUNTER` go up at 32768 per second. That counter is our clock.
4. Change the advertising interval (`BT_LE_ADV_CONN_FAST_1` -> `_2`) and see the scan
   speed change in the app.
5. Later, optional: a **BLE sniffer** (nRF Sniffer + Wireshark) needs a second nRF board or a dongle.
   It shows every radio packet. Only if you want to go deeper.
