# ble-watch

A minimal Bluetooth LE watch with a binary LED face. It is a pet project, built mostly by AI agents, and its main goal is **learning**: AI-driven development, Zephyr, nRF MCUs and embedded hardware.

## Scope (keep it minimal)
- Show the time on 11 yellow LEDs in binary (24-hour time, no AM/PM LED).
- No buttons (for waterproofing). The watch wakes on a **double-tap**, detected by the accelerometer, shows the time for a few seconds, then turns the LEDs off.
- Set the time from an Android phone with the generic **nRF Connect** app (no custom app).
- Remote firmware updates over BLE (MCUboot + SMP, **nRF Connect Device Manager** app).
- Out of scope: notifications, fitness, step counting, custom phone apps. Don't add features that aren't in the plan.

See `docs/PLAN.md` for the steps and the current step.

## Hardware
- Development board: **nRF52840 DK** (PCA10056). Zephyr board target: `nrf52840dk/nrf52840`.
- Display: 11 yellow LEDs, each on its own GPIO through a resistor, about 1 mA each. Layout:
  ```
  Hours:        16   8   4   2   1     (0–23)
  Minutes:  32  16   8   4   2   1
  ```
  On the DK, the LEDs sit on a breadboard, wired to the Arduino header.
- Accelerometer: ST **LIS2DW12** on I²C plus one interrupt line (INT1). Zephyr compatible `st,lis2dw12`. On the DK, use a breakout board on the Arduino header I²C (`arduino_i2c`).
- Final watch: bare nRF52840 on a custom PCB, nPM1100 charger, 4 mm LiPo, 3D-printed case. Not needed until after step 4. See `docs/hardware.md`.
- Record the final pinout in `docs/hardware.md` and in the devicetree overlay.

## Toolchain
- nRF Connect SDK (NCS), installed via nRF Connect for VS Code. Pin the exact NCS version in `docs/hardware.md` once it's installed.
- Build: `west build -b nrf52840dk/nrf52840 app` (pristine build: add `-p`)
- Flash: `west flash`
- Logs: RTT or the DK's USB serial (115200 baud).

## Layout
```
app/            Zephyr application (CMakeLists.txt, prj.conf, src/, boards/*.overlay)
docs/PLAN.md    plan, step status, learning notes
docs/hardware.md  parts, pinout, versions, measurements
docs/learn/     one short note per concept learned
```

## How to work in this repo
- **Small steps.** One task at a time. Each task ends with something the user can see on the DK (LED, log line) and a commit.
- **Build before you claim success.** Run the build and show the result. If you can't flash or see the hardware, say so and ask the user to check.
- **Ask before flashing** or touching anything that changes the hardware state, and before adding a dependency.
- Prefer Zephyr/NCS APIs and existing samples over hand-written drivers. Configure through devicetree and Kconfig, not hard-coded pins. Describe the 11 LEDs in devicetree (e.g. a `gpio-leds` node) and drive them with the GPIO/LED APIs.
- **Power matters.** It's a watch on a small battery: the LEDs are off by default, and the CPU sleeps between events. Don't add busy loops or polling.
- C (C11), Zephyr coding style, `LOG_MODULE_REGISTER` per file, no dynamic allocation unless needed.
- Commits: short imperative subject (e.g. "Add LIS2DW12 overlay").

## Learning mode (important)
The user is new to Zephyr and nRF. After each task:
1. Explain the key concepts used, briefly and in plain words (e.g. what the overlay does, why this Kconfig option is needed).
2. Point to the relevant file and lines.
3. Ask 2–3 short questions to check understanding, and don't give the answers unless asked.
4. Add or update a note in `docs/learn/<topic>.md`.

The user is also improving their English: at the end of each reply, add a short **"Feedback & grammar"** section that fixes their last message.
