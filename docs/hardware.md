# Hardware

## Main parts (final watch)

Decided 2026-11-09. This list has the main components only. Supporting parts (antenna, crystals, resistors, capacitors, inductors, ESD and protection diodes) come from the reference designs and will be added during the PCB design.

| # | Part | Part number | Package | Notes |
|---|------|-------------|---------|-------|
| 1 | MCU + BLE radio | Nordic **nRF52840** | aQFN73 (7×7 mm) | Bare chip, the same SoC as the nRF52840 DK. Follow Nordic's reference layout. Order assembled boards (JLCPCB/PCBWay). |
| 2 | Charger + regulator | Nordic **nPM1100** | QFN or WLCSP | LiPo charger plus a 3.0 V buck regulator. Has a ship mode. |
| 3 | Accelerometer | ST **LIS2DW12** | LGA-12, 2×2×0.7 mm | Wakes the watch on double-tap. I²C + INT1 (3 GPIOs). Zephyr driver `st,lis2dw12`. |
| 4 | LEDs ×11 | Lite-On **LTST-C190KSKT** (yellow) | 0603 | 5 for hours (16..1, 24-hour time, top row), 6 for minutes (32..1, bottom row). About 1 mA each, driven straight from a GPIO. |
| 4b | Status LEDs ×2 | Lite-On **LTST-C190KRKT** (red), **LTST-C190KGKT** (yellow-green) | 0603 | Heartbeat flash in the battery colour. Both AlInGaP, so the same 10 kΩ fits both (≈0.12 mA). |
| 4c | Battery divider | 2 × 1 MΩ (YAGEO RC0402FR-071ML) + 100 nF | 0402 | VBAT / 2 to the SAADC (AIN1). Always on, 2.1 µA. |
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

## Ship mode

Hardware is wired; the firmware is not written yet and not in `docs/PLAN.md`.

- **What:** the nPM1100 turns off VOUT and disconnects the system from the battery (well under 1 µA left), for storage and shipping. The nRF loses power, so the RTC time is lost and is set again from the phone after wake-up.
- **Wiring: one GPIO is enough.** nRF **P0.15 → SHPACT** (internal 500 kΩ pull-down, so it stays low by itself). CHG (P0.11) and ERR (P1.09) are optional status lines; ship mode doesn't need them.
- **Enter:** charger unplugged → turn the LEDs off → drive SHPACT high for longer than the ship-mode delay (>200 ms in `hw/power.ato`, **VERIFY in the nPM1100 PS**) → power disappears. If the code still runs after the delay, the request was refused (cable probably plugged in): set the pin low again and log a warning.
- **Exit:** plug in the charger (VBUS). SHPHLD (the usual wake button) has no button here; see open question 2 in `hw/README.md` (internal pull-up, or tie it high).
- **Limit:** no VBUS-sense line reaches the nRF, and CHG goes high again when the battery is full, so the firmware can't know for sure whether the cable is in. It tries, then checks.
- **Later firmware:** pin in the watch overlay as `zephyr,user { ship-gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>; };`, read with `GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), ship_gpios)`. Possible trigger: a BLE write from nRF Connect.

## Firmware updates

- First flash: over SWD (Tag-Connect pads), with the DK as the programmer.
- After that: OTA only, with MCUboot + SMP through the nRF Connect Device Manager app on Android.

## Development setup

- nRF52840 DK (PCA10056), Zephyr board target `nrf52840dk/nrf52840`
- LIS2DW12 breakout board on the DK's I²C (Arduino header). Tested 2026-11-21: double-tap works.
  The breakout ties SDO/SA0 low (address 0x18) and leaves CS unconnected, so CS is wired to VDD by hand.
- 11 yellow LEDs plus resistors on a breadboard to test the binary display layout
- NCS v3.4.2
- Pinout: below

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

Accelerometer (LIS2DW12 breakout, node `accel` in the same overlay, tested and working):

| Signal | DK connection | nRF pin | Notes |
|--------|---------|---------|-------|
| SDA  | D14 | P0.26 | `arduino_i2c` (i2c0). Pull-ups needed; the DK has none. |
| SCL  | D15 | P0.27 | |
| INT1 | D2  | P1.03 | Active high. Uses GPIO SENSE (`sense-edge-mask`), not GPIOTE. |
| VCC  | VDD | —     | 3.0 V on the DK. |
| GND  | GND | —     | |
| SDO/SA0 | —   | —     | Low on the breakout (measured) = I²C address 0x18. High would be 0x19. |
| CS   | VDD | —     | Wired to VDD by hand: the breakout leaves it open. CS high = I²C mode, low = SPI. |

Heartbeat LEDs (red, green) and battery (nodes `status_led` and `vbatt` in the same overlay). Not tested on hardware yet.
The board's own `leds` node is disabled in the overlay, so only `status_led` drives P0.13 and P0.14.

| Signal | DK connection | nRF pin | Notes |
|--------|---------|---------|-------|
| Status red   | DK LED2 | P0.14 | Active low (the DK LED is wired from VDD to the pin). Green on the DK; red on the watch. |
| Status green | DK LED1 | P0.13 | Active low. On the watch: active high, low brightness set by the resistor (no PWM). |
| Battery      | A0 | P0.03 = AIN1 | Final watch: VBAT -[1 MΩ]- AIN -[1 MΩ]- GND, 100 nF AIN to GND (2.1 µA always). DK: a potentiometer between VDD and GND, wiper on A0, stands in for VBAT/2 (1.8–2.1 V on A0 = 3.6–4.2 V battery). |

## Watch pinout (draft, from `hw/`)

Source of truth: `hw/main.ato` (also in `hw/README.md`).

**Rule: every signal that exists on the DK uses the same nRF pin on the watch.** What works on the
breadboard is what the PCB connects, and the pin numbers in the DK overlay carry over unchanged.
The watch still gets its own board files in Step 5 (no Arduino header names, active-high status LEDs,
nPM1100 pins).

| Function | nRF pin | DK | Notes |
|----------|---------|----|-------|
| Hours 16, 8, 4, 2, 1 | P1.04, P1.05, P1.06, P1.07, P1.08 | D3–D7 | GPIO → 1 kΩ → LED → GND, active high (as on the breadboard). |
| Minutes 32, 16, 8, 4, 2, 1 | P1.10, P1.11, P1.12, P1.13, P1.14, P1.15 | D8–D13 | |
| I²C SDA / SCL | P0.26 / P0.27 | D14 / D15 | 4.7 kΩ pull-ups on the board. |
| Accel INT1 | P1.03 | D2 | GPIO SENSE (`sense-edge-mask` bit 3 of gpio1), as on the DK. |
| Status green / red | P0.13 / P0.14 | LED1 / LED2 | GPIO → 10 kΩ → LED → GND, so **active high** (the DK LEDs are active low). |
| Battery sense (VBAT / 2) | P0.03 = AIN1 | A0 | VBAT -[1 MΩ]- AIN1 -[1 MΩ]- GND, 100 nF AIN1 to GND. Same `channel@0` config. |
| Accel INT2 | P0.12 | — | Watch only. Spare, the firmware doesn't use it. |
| Charger CHG / ERR | P0.11 / P1.09 | — | Watch only. Open drain, internal pull-ups. |
| Ship mode (out) | P0.15 | — | Watch only. nPM1100 SHPACT. See "Ship mode" above. |
| 32.768 kHz crystal | P0.00 / P0.01 | (same) | |
| SWD + reset | SWDIO, SWDCLK, P0.18 | — | Tag-Connect pads. |

Layout notes:
- The hour pins (P1.04–P1.07) and INT1 (P1.03) sit on the same side of the chip as the antenna pin (ANT).
  They carry only slow signals (1 kHz PWM at ~1 mA, a rare interrupt), and the DK uses the same pins next
  to its antenna, but keep their traces away from the RF path and the antenna.
- Watch-only signals can't be tested on the breadboard: check them against the nPM1100 datasheet.
