# Hardware

## Main parts (final watch)

Decided 2026-11-09. This list has the main components only. Supporting parts (antenna, crystals, resistors, capacitors, inductors, ESD and protection diodes) come from the reference designs and will be added during the PCB design.

| # | Part | Part number | Package | Notes |
|---|------|-------------|---------|-------|
| 1 | MCU + BLE radio | Nordic **nRF52840** | aQFN73 (7×7 mm) | Bare chip, the same SoC as the nRF52840 DK. Follow Nordic's reference layout. Order assembled boards (JLCPCB/PCBWay). |
| 2 | Charger + regulator | Nordic **nPM1100** | QFN or WLCSP | LiPo charger plus a 3.0 V buck regulator. Has a ship mode. |
| 3 | Accelerometer | ST **LIS2DW12** | LGA-12, 2×2×0.7 mm | Wakes the watch on double-tap. I²C + INT1 (3 GPIOs). Zephyr driver `st,lis2dw12`. |
| 4 | LEDs ×11 | Lite-On **LTST-C190KSKT** (yellow) | 0603 | 5 for hours (16..1, 24-hour time, top row), 6 for minutes (32..1, bottom row). About 1 mA each, driven straight from a GPIO. |
| 5 | Battery | LiPo **402025**, about 150 mAh, with a built-in protection board (PCM) | 4×20×25 mm | |
| 6 | Charging connector | 2-pin magnetic pogo connector (flat half in the watch) | — | Carries charger input (VBUS) only, never the battery. |
| 7 | Programming | Tag-Connect **TC2030-NL** footprint (SWD) | PCB pads only | For the first flash, using the DK as the programmer. Then OTA. |

## Display layout

```
Hours:        16   8   4   2   1     (0–23)
Minutes:  32  16   8   4   2   1
```

## Case

Decided 2026-11-08. Two 3D-printed parts: a **clear SLA resin top** (the PCB is visible through it) and a coloured bottom tray, joined by **4 × M1.2 screws** from below, with a gasket in the joint. Nearly waterproof, with no buttons and no openings except the charging pads. About the size of a small retro Casio.

## Charging and protection

- The charger (nPM1100) is inside the watch, so the pads only carry 5 V in from the cable, never the battery.
- A low-Vf Schottky diode in series keeps the pads at 0 V when unplugged, so a short (sweat, a key) can't drain the cell and the pads don't corrode.
- A 5 V TVS diode on VBUS protects against static.
- Polarity-keyed magnets let the cable snap on only one way.
- The cell's own protection board (PCM) is the last safety layer.

## Firmware updates

- First flash: over SWD (Tag-Connect pads), with the DK as the programmer.
- After that: OTA only, with MCUboot + SMP through the nRF Connect Device Manager app on Android.

## Development setup

- nRF52840 DK (PCA10056), Zephyr board target `nrf52840dk/nrf52840`
- LIS2DW12 breakout board on the DK's I²C (Arduino header). The exact board is still to be chosen.
- 11 yellow LEDs plus resistors on a breadboard to test the binary display layout
- NCS v3.4.2
- Pinout: below (the accelerometer part is still to be chosen)

## DK pinout (step 1)

LED wiring: pin → resistor → LED anode, LED cathode → GND. A high pin lights the LED.
Each LED is one PWM channel; set in `app/boards/nrf52840dk_nrf52840.overlay`.

| LED | Value | Arduino | nRF pin | PWM |
|-----|-------|---------|---------|-----|
| H16 | 16 | D3  | P1.04 | pwm1 ch0 |
| H8  | 8  | D4  | P1.05 | pwm0 ch3 |
| H4  | 4  | D5  | P1.06 | pwm0 ch2 |
| H2  | 2  | D6  | P1.07 | pwm0 ch1 |
| H1  | 1  | D7  | P1.08 | pwm0 ch0 |
| M32 | 32 | D8  | P1.10 | pwm2 ch2 |
| M16 | 16 | D9  | P1.11 | pwm2 ch1 |
| M8  | 8  | D10 | P1.12 | pwm2 ch0 |
| M4  | 4  | D11 | P1.13 | pwm1 ch3 |
| M2  | 2  | D12 | P1.14 | pwm1 ch2 |
| M1  | 1  | D13 | P1.15 | pwm1 ch1 |

Kept free for the accelerometer: D14/D15 (P0.26/P0.27, `arduino_i2c` SDA/SCL) and D2 (P1.03, planned for INT1).
DK LED1 (P0.13) stays the GPIO heartbeat.
