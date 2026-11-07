# Hardware

## Board
- nRF52840 DK (PCA10056), Zephyr target `nrf52840dk/nrf52840`.
- On-board debugger: J-Link OB-nRF5340, serial number 1050265215.
- Serial console: the DK's VCOM0 (on this PC it was `/dev/ttyACM2`; the number can change).

## Versions
- J-Link software: V8.94
- NCS: v3.4.2 (Zephyr 4.4.2), installed manually from GitHub in `~/ncs/v3.4.2`
  (Nordic's download server blocks our IP, so the nRF Connect SDK Manager can't be used).
- Python venv with west 1.5.0: `~/ncs/v3.4.2/.venv`
- Zephyr SDK 1.0.1 (arm-zephyr-eabi-gcc 14.3.0), ARM toolchain only: `~/zephyr-sdk-1.0.1`
- Flash runner: `jlink` (nrfutil's core isn't installed)

## Display
- Module: 1.54" e-ink, 200x200, black/white, sold as "XSRUPB" (label E154A07N193G02).
- FPC (ribbon) marking: `HINK-E0154A07-A1`, which is the Good Display **GDEH0154D67** panel.
- Controller: **SSD1681** (Zephyr compatible `gooddisplay,gdeh0154a07`, `solomon,ssd1681`, driver `ssd16xx`).
  Source: Zephyr's Waveshare e-Paper shield docs map HINK-E0154A07 to GDEH0154D67 / SSD1681.
- It is a raw panel with a 24-pin 0.5 mm FPC. It needs a driver board (boost circuit + FPC connector) before it can connect to the DK.
- Zephyr already has a shield for it: `waveshare_epaper_gdeh0154a07` (uses `arduino_spi`).

## Display driver board
- Ordered: **Waveshare e-Paper Driver HAT** (Raspberry Pi form factor). It does not plug onto the DK, so it is wired with jumper wires to the Arduino header pins that the Zephyr shield `waveshare_epaper_gdeh0154a07` expects.
- HAT switches: Display Config = **A (3R)** for the 1.54" panel (check the HAT wiki), Interface Config = **0 (4-line SPI)**.

## Display pinout (planned, from the Zephyr shield overlay + DK devicetree)
| HAT pin | Function       | DK Arduino pin | nRF52840 pin |
|---------|----------------|----------------|--------------|
| VCC     | 3.3 V supply   | VDD (3V)       | -            |
| GND     | ground         | GND            | -            |
| DIN     | SPI MOSI       | D11            | P1.13        |
| CLK     | SPI SCK        | D13            | P1.15        |
| CS      | chip select    | D10            | P1.12        |
| DC      | data/command   | D9             | P1.11        |
| RST     | reset (low)    | D8             | P1.10        |
| BUSY    | busy (high), internal pull-down (app overlay) | D7 | P1.08 |
| PWR     | power enable (only on newer HATs) | tie to VDD | - |

Verified in the generated `zephyr.dts` of a test build (NCS v3.4.2). Not yet tested on hardware.
