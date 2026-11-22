# Battery level with the SAADC, and the heartbeat LEDs

- **SAADC** = the nRF52840's analog-to-digital converter. It measures a pin voltage against a
  reference. We use the internal 0.6 V reference with gain 1/6, so the full scale is 3.6 V.
- The battery (up to 4.2 V) is higher than the chip's supply (3.0 V), so it can't go straight
  to a pin. A **resistor divider** (1 MΩ + 1 MΩ) halves it. Big resistors waste little current
  (2.1 µA), but the ADC then needs a long **acquisition time** (40 µs) to charge its sample
  capacitor through them; the 100 nF capacitor on the pin helps too.
- Zephyr's **`voltage-divider`** binding describes this: `io-channels` points at the ADC channel,
  `output-ohms` / `full-ohms` give the ratio. Its sensor driver returns the battery voltage
  through the normal sensor API (`SENSOR_CHAN_VOLTAGE`), so `battery.c` has no ADC maths.
- The ADC channel is a child node of `&adc` (`channel@0`): gain, reference, acquisition time,
  input pin (`NRF_SAADC_AIN1` = P0.03 = Arduino A0), resolution.
- The red and green LEDs are a **`gpio-leds`** node: plain on/off GPIOs behind the same LED API as the face
  (`led_on()`, `led_off()`). Brightness comes from the resistors, not from PWM.
- Heartbeat, once a minute: **green** flash = battery OK, **red** flash = below
  `CONFIG_BATTERY_LOW_MV`; back to green only at `CONFIG_BATTERY_LOW_MV +
  CONFIG_BATTERY_HYSTERESIS_MV` (**hysteresis**: two thresholds, so a battery sitting right at
  the limit doesn't switch colour every minute). If a reading fails, the last colour stays and
  the log shows the error. A floating A0 pin is *not* an error: it gives a random voltage.
- On a **double-tap**, the battery is measured *before* the face LEDs turn on (their ~11 mA
  would pull the voltage down), and the status LED stays lit together with the face. The
  periodic flash pauses while the face is lit and starts a new full period when it turns off.
- `heartbeat.c` uses one **delayable work item** that reschedules itself, one step per run
  (measure + LED on, then LED off and wait for the next minute). Nothing runs in between, so the
  CPU sleeps.
- Files: `app/boards/nrf52840dk_nrf52840.overlay` (end), `app/src/battery.c`,
  `app/src/heartbeat.c`, options in `app/Kconfig`.

## SAADC in more detail

- **SAADC = Successive Approximation ADC.** It finds the voltage by a binary search: an internal
  DAC makes a test voltage, a comparator says "higher" or "lower", and each step fixes one bit,
  from the top bit down. 12 bits = 12 comparisons. Before that, a small capacitor samples the
  input during the **acquisition time** (that's why a high-resistance source needs a longer one).
- nRF52840 SAADC: 8 analog pins (AIN0–AIN7 = P0.02–P0.05, P0.28–P0.31) plus internal VDD and
  VDDH/5; 8/10/12-bit (14 with oversampling); up to 200 k samples/s; results go to RAM by EasyDMA.
- **Reference** = the voltage the input is compared with. The internal one is **0.6 V**, from a
  bandgap circuit inside the chip, so it doesn't change when VDD changes (battery, coin cell).
  It's small because the chip must work from supplies as low as 1.7 V. The other option is
  VDD/4, which moves with the supply.
- **Gain** scales the input *before* the comparison. Full scale = reference / gain:

  | Gain | 1/6 | 1/5 | 1/4 | 1/3 | 1/2 | 1 | 2 | 4 |
  |---|---|---|---|---|---|---|---|---|
  | Full scale (0.6 V ref) | 3.6 V | 3.0 V | 2.4 V | 1.8 V | 1.2 V | 0.6 V | 0.3 V | 0.15 V |

- Formula (single-ended): `RESULT = V_pin × GAIN / REFERENCE × 2^12`.
  Example: battery 3.9 V → pin 1.95 V → × 1/6 = 0.325 V → 0.325 / 0.6 × 4096 ≈ **2218**.
  The driver goes back: 2218 / 4096 × 3.6 V × 2 (divider) ≈ 3.90 V.
- A 3.6 V range doesn't allow 3.6 V on the pin: a pin must never go above VDD (+0.3 V).

## On the watch PCB (`hw/status.ato`)

- Same circuit as the firmware expects: VBAT -[1 MΩ]- AIN1 (P0.03) -[1 MΩ]- GND, 100 nF on AIN1.
  Keeping the **same pin and resistor values** as the DK means the `vbatt` node and `channel@0`
  can be copied unchanged into the watch's own overlay.
- The divider sits on the **battery** (before the nPM1100), not on the 3.0 V rail: the rail is
  regulated, so it would read 3.0 V until the battery is almost empty.
- Status LEDs are **active high** on the watch (GPIO → resistor → LED → GND) but **active low** on
  the DK. Only the overlay flag (`GPIO_ACTIVE_HIGH` / `GPIO_ACTIVE_LOW`) changes; `heartbeat.c` stays
  the same, because `led_on()` hides the polarity.
- Red and yellow-green AlInGaP LEDs have almost the same forward voltage (~1.75 V at 0.1 mA), so
  one resistor value (10 kΩ → ≈0.12 mA) gives both a similar current. A "true" InGaN green
  (Vf ≈ 2.7 V) would leave only ~0.3 V across the resistor at 3.0 V: dim and uneven.
