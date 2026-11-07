# Toolchain: the pieces and what each one does

- **nRF Connect SDK (NCS)**: Nordic's SDK. It is a set of git repos: Nordic's fork of
  **Zephyr** (the RTOS), `nrf` (Nordic libraries and samples), MCUboot, and others.
- **west**: Zephyr's meta-tool. It does two jobs:
  1. Manages the repos (`west init`, `west update` read a manifest file, `west.yml`).
  2. Runs commands: `west build` (CMake + Ninja under the hood), `west flash`, `west debug`.
- **Toolchain bundle**: the compiler (`arm-zephyr-eabi-gcc`), CMake, Ninja, `dtc`, Python
  packages. Nordic ships a matching bundle for each NCS version, so versions don't clash.
- **nrfutil**: Nordic's command-line tool, with plugins. `nrfutil device list` finds boards;
  `nrfutil toolchain-manager` / `sdk-manager` install the SDK. It replaces the old `nrfjprog`.
- **SEGGER J-Link**: the debug probe software. The DK has a J-Link chip on board (the
  "interface MCU"), so flashing goes: west → nrfutil → J-Link → nRF52840.

The DK has two USB ports. The one marked **USB** (the "interface" port, on the short edge) is
the debugger + serial console. The **nRF USB** port is the nRF52840's own USB, used later only
if the app enables it.

How to check a DK is connected on Linux: `lsusb | grep -i segger` (vendor ID `1366`).
## What Linux sees when the DK is plugged in
- `lsusb` → `1366:1051 SEGGER J-Link`. `JLinkExe` → `ShowEmuList` shows the probe name
  `J-Link OB-nRF5340-NordicSemi`: on newer DKs the on-board debugger is itself an nRF5340 chip.
- It creates **two serial ports** (`/dev/ttyACM*`), one per VCOM. Find which ones are the DK's:
  `udevadm info -q property -n /dev/ttyACM2 | grep -E 'ID_VENDOR|ID_SERIAL_SHORT'`.
  The port with the lower USB interface number (VCOM0) is wired to the nRF52840's UART0,
  which is where Zephyr's console and logs go by default (115200 baud).
- `/dev/ttyACM*` numbers are not stable. They depend on what was plugged in first, so check
  each time instead of hard-coding a number.
- To open the ports without `sudo`, your user must be in the `dialout` group (`groups`).

## Building a freestanding app (our repo lives outside `~/ncs/v3.4.2`)
- `west build` is a Zephyr *extension* command: west only knows it when it finds a workspace.
  From outside the workspace, west finds it through `ZEPHYR_BASE`. Without it you get
  `west: unknown command "build"`.
- The `ncs` alias in `~/.zshrc` activates the venv and sets `ZEPHYR_BASE` and `ZEPHYR_SDK_INSTALL_DIR`:
  `ncs`, then `west build -p -b nrf52840dk/nrf52840 app` from the repo root.
