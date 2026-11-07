# ble-watch

A minimal Bluetooth LE e-ink watch. It is a pet project, built mostly by AI agents, and its main goal is **learning**: AI-driven development, Zephyr, nRF MCUs and embedded hardware.

## Scope (keep it minimal)
- Show the time (HH:MM) on the e-ink display.
- Set the time from an Android phone with the generic **nRF Connect** app (no custom app).
- Remote firmware updates over BLE (MCUboot + SMP, **nRF Connect Device Manager** app).
- Out of scope: notifications, fitness, sensors, custom phone apps. Don't add features that aren't in the plan.

See `docs/PLAN.md` for the steps and the current step.

## Hardware
- Board: **nRF52840 DK** (PCA10056). Zephyr board target: `nrf52840dk/nrf52840`.
- Display: 1.54" e-ink module "XSRUPB", 200x200, SPI. The controller is probably SSD1681 (Zephyr compatible `solomon,ssd1681`, driver `ssd16xx`). **Confirm from the module markings or the seller's datasheet before relying on this.**
- Wiring: use the DK's Arduino header SPI (`arduino_spi`). Display pins: CS, DC, RST, BUSY, plus 3.3 V and GND. Record the final pinout in `docs/hardware.md` and in the devicetree overlay.

## Toolchain
- nRF Connect SDK (NCS), installed via nRF Connect for VS Code. Pin the exact NCS version in `docs/hardware.md` once it's installed.
- Build: `west build -b nrf52840dk/nrf52840 app` (pristine build: add `-p`)
- Flash: `west flash`
- Logs: RTT or the DK's USB serial (115200 baud).

## Layout
```
app/            Zephyr application (CMakeLists.txt, prj.conf, src/, boards/*.overlay)
docs/PLAN.md    plan, step status, learning notes
docs/hardware.md  pinout, versions, measurements
docs/learn/     one short note per concept learned
```

## How to work in this repo
- **Small steps.** One task at a time. Each task ends with something the user can see on the DK (LED, log line, pixels) and a commit.
- **Build before you claim success.** Run the build and show the result. If you can't flash or see the hardware, say so and ask the user to check.
- **Ask before flashing** or touching anything that changes the hardware state, and before adding a dependency.
- Prefer Zephyr/NCS APIs and existing samples over hand-written drivers. Configure through devicetree and Kconfig, not hard-coded pins.
- C (C11), Zephyr coding style, `LOG_MODULE_REGISTER` per file, no dynamic allocation unless needed.
- Commits: short imperative subject (e.g. "Add SSD1681 display overlay").

## Learning mode (important)
The user is new to Zephyr and nRF. After each task:
1. Explain the key concepts used, briefly and in plain words (e.g. what the overlay does, why this Kconfig option is needed).
2. Point to the relevant file and lines.
3. Ask 2–3 short questions to check understanding, and don't give the answers unless asked.
4. Add or update a note in `docs/learn/<topic>.md`.

The user is also improving their English: at the end of each reply, add a short **"Feedback & grammar"** section that fixes their last message.
