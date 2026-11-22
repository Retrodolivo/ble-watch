# Watch hardware (atopile)

Schematic-as-code for the BLE binary watch, written in [atopile](https://atopile.io) `.ato`.
Status: **first draft**. It compiles and produces a KiCad PCB with all 76 parts and nets
(nothing placed or routed yet). Every value marked VERIFY must be checked before ordering.

## Build

```bash
uv tool install --python 3.14 "atopile==0.15.8"   # 0.12.x is retired; 0.16+ is the web app
cd hardware/watch-hw            # wherever you put this folder
ato build                       # -> layouts/default/default.kicad_pcb, build/builds/default/*
```

Open `layouts/default/default.kicad_pcb` in KiCad 9 to see the parts and the ratsnest.
atopile is code-first: there is no drawn schematic sheet. The `.ato` files *are* the schematic.

## Files

| File | What it describes |
|------|-------------------|
| `main.ato` | Top level `Watch`: wires the blocks together, GPIO map |
| `power.ato` | Pogo pads, TVS + Schottky, nPM1100 charger/buck (3.0 V), battery pads |
| `mcu.ato` | nRF52840 aQFN73, decoupling, DC/DC, 32 MHz + 32.768 kHz crystals, RF match, chip antenna, Tag-Connect SWD |
| `accel.ato` | LIS2DW12 on I2C (0x18) with INT1/INT2 and pull-ups |
| `display.ato` | 11 yellow LEDs (5 hours, 6 minutes), each GPIO -> 1 k -> LED -> GND |
| `status.ato` | Red + green heartbeat LEDs (GPIO -> 10 k -> LED -> GND), VBAT divider (1 M + 1 M, 100 nF) to AIN1 |
| `parts/` | One folder per physical part: `.ato` + KiCad footprint + symbol |
| `tools/genparts.py` | Script that generated `parts/` from the official KiCad libraries |

## GPIO map

Every signal that also exists on the DK uses the same nRF pin as the DK breadboard
(`app/boards/nrf52840dk_nrf52840.overlay`). Only INT2, CHG, ERR and ship mode are watch-only.

| Function | nRF52840 pin |
|----------|--------------|
| Hours 16, 8, 4, 2, 1 | P1.04, P1.05, P1.06, P1.07, P1.08 |
| Minutes 32, 16, 8, 4, 2, 1 | P1.10, P1.11, P1.12, P1.13, P1.14, P1.15 |
| I2C SDA / SCL | P0.26 / P0.27 |
| Accel INT1 / INT2 | P1.03 / P0.12 |
| Charger CHG / ERR (open drain, use internal pull-up) | P0.11 / P1.09 |
| Ship mode (output) | P0.15 |
| Status LED green / red (active high) | P0.13 / P0.14 |
| Battery sense (VBAT / 2) | P0.03 = AIN1 |
| 32.768 kHz crystal | P0.00 / P0.01 |
| SWD + reset | SWDIO, SWDCLK, P0.18 |

## Parts and LCSC numbers

Every part is a fixed manufacturer part number with its own local footprint, and every part that gets
assembled has an LCSC number (`supplier_partno` in `parts/*/*.ato`). `ato build` writes a complete
JLCPCB BOM to `build/builds/default/default.bom.csv` (28 lines, 71 parts). The 4 pogo/battery pads and
the Tag-Connect footprint are bare copper with no part, so their LCSC number is `none` and the JLC BOM
leaves them out. The build prints 5 "Non-LCSC parts" warnings for them; these are expected.

We don't use atopile's cloud picker or `ato create part`. Both depend on atopile's components API,
which returned HTTP 502 on 2026-11-18 (atopile 0.15.8 is the last CLI release and is in maintenance mode).
Fixed parts also keep the BOM deterministic.

Three parts differ from Nordic's reference Murata parts, which LCSC doesn't stock. Same size and value:

| Part | Reference part | Used (LCSC) | Check before ordering |
|------|-----|-----------|-----------------------|
| `C_12pF_0402` | GRM1555C1H120JA01D | GJM1555C1H120GB01D (C161305) | High-Q GJM series, ±2 %. Drop-in |
| `L_3n9H_0402` | LQP15MN3N9B02D (obsolete) | LQG15HH3N9B02D (C907911) | Multilayer, not thin-film: RF match may need retuning |
| `L_10uH_0603` | LQM18PN100MFRL | LQM18DZ100M70L (C3911082) | Check rated current and DCR for the nRF52840 DC/DC |

When you change a part, update its `supplier_partno` and the matching row in `tools/genparts.py`, so
regenerating `parts/` keeps the number. LCSC numbers were looked up on 2026-11-18. Check stock and
lifecycle before ordering: several of the Murata MLCCs are marked NRND by other distributors.

## Open questions (decide before layout)

1. **nPM1100 QFN pinout and exposed-pad size** come from LCSC/Flux data, not Nordic's own QFN spec. Verify, and pick the
   matching footprint (currently `QFN-24-1EP_4x4mm_P0.5mm_EP2.6x2.6mm`). The WLCSP version is smaller but harder to assemble.
2. **Ship mode without buttons.** Firmware can drive SHPACT to enter ship mode (for storage); plugging the charger wakes it.
   SHPHLD is left unconnected. Check that it has an internal pull-up, or tie it high.
3. **nRF52840 passive values** (DEC1/3/4/6, DECUSB, VDD caps, the 10 uH + 15 nH DC/DC pair, 12 pF load caps) follow
   Nordic's reference circuit from memory. Check them against the nRF52840 PS "Reference circuitry" (aQFN73, DC/DC enabled).
4. **Antenna tuning.** Nordic's 3.9 nH / 0.8 pF filter is followed by a pi network (0 R + two DNP shunts) for tuning the
   2450AT18A100 on the real board with a VNA. Keep the antenna in a corner with the ground clearance from Johanson's layout.
5. **Charge-pad protection.** The pads see only 5 V in, through a 6 V bidirectional TVS and a series Schottky (RB521S30),
   which also blocks a reversed cable. The Schottky drops ~0.4 V, which leaves ~4.6 V at VBUS. The TVS footprint
   is a generic 0402 and must be checked against TI's DPY land pattern.
6. **Charge current** is 100 mA (ICHG = 4.7 k, about 0.66 C for 150 mAh). Check the cell's maximum charge rate.
7. **Battery level.** An always-on VBAT divider (1 M + 1 M, 2.1 uA) goes to P0.03/AIN1, the same SAADC input the
   firmware uses on the DK. A switched divider (a FET to cut the 2.1 uA)
   is possible but not worth it: the chip sleeps at a similar current. The 100 nF must stay close to the pin.
8. **LED brightness.** 1 mA through clear resin may be too bright or too dim. The resistors (1 k) are easy to change.
   The status LEDs run at ~0.12 mA (10 k); check that the flash is visible.
