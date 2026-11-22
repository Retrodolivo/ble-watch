# Ship mode (nPM1100)

- **Ship mode** = the PMIC disconnects the whole system from the battery. Only the PMIC's own tiny
  leakage remains (well under 1 µA), so a product can sit in a box for months without draining the cell.
  Normal sleep still costs a few µA (RTC, accelerometer, battery divider).
- **Enter:** the MCU holds the nPM1100's **SHPACT** pin high for a short delay (charger unplugged).
  Power to the MCU then simply disappears; nothing in the code runs after that.
- **Exit:** VBUS (the charger) or the SHPHLD pin (usually a button; this watch has none). The MCU
  boots from scratch, so RAM and the RTC time are lost.
- **Try, then check:** the watch can't see VBUS, so the firmware can't know in advance if the
  request will be accepted. It sends the pulse, waits, and if it's still running, it knows ship mode
  was refused.
- **`zephyr,user`** is a devicetree node for one-off pins that have no driver of their own:
  `ship-gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>;` then `GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), ship_gpios)`
  in C. No hard-coded pin numbers.
- Files: `hw/power.ato` (SHPACT), `hw/main.ato` (P0.15), `docs/hardware.md` (section "Ship mode").
