# LIS2DW12, I²C, sensor triggers and double-tap (step 1)

## Pieces
- **I²C**: a two-wire bus (SDA data, SCL clock) with pull-up resistors. Each chip has an address;
  the LIS2DW12 is 0x19 or 0x18 (set by its SA0 pin). The DK already enables `arduino_i2c`
  (= `i2c0`, P0.26/P0.27), so the overlay only adds a child node `accel: lis2dw12@18`.
  `@18` and `reg = <0x18>` must match. SA0 is the same pin as SDO (used for SPI); ours is low → 0x18.
- **Driver**: Zephyr has one (`zephyr/drivers/sensor/st/lis2dw12/`). It turns on by itself
  (`CONFIG_LIS2DW12`, `CONFIG_I2C`) because a node with `compatible = "st,lis2dw12"` exists.
  At boot it reads the WHO_AM_I register; if that fails, `device_is_ready()` is false.
- **Settings in devicetree**: `odr`, `range`, `tap-*` come from the binding
  (`zephyr/dts/bindings/sensor/st,lis2dw12-common.yaml`). The driver writes them to the chip at boot.

## How a double-tap reaches our code
1. The chip samples at 400 Hz and recognises the double-tap **by itself**. The CPU sleeps.
2. It raises INT1 (D2 / P1.03). The GPIO interrupt wakes the CPU.
3. The driver's ISR only submits work to the **system work queue** (`CONFIG_LIS2DW12_TRIGGER_GLOBAL_THREAD`).
   I²C can't run in an ISR, so reading the interrupt source happens in that thread.
4. It calls our handler, set with `sensor_trigger_set()` (`app/src/accel.c`).
5. `on_double_tap()` in `app/src/main.c` lights the time and schedules `face_off_work`
   (a `k_work_delayable`) 5 s later. A new tap restarts the 5 s.

## Tap timing (ST AN5038 values, at 400 Hz)
| Setting | Value | Time | Meaning |
|---------|-------|------|---------|
| threshold | 12 | 750 mg | how hard a tap must be (per axis, 0 = axis off) |
| shock | 3 | 60 ms | max length of one tap |
| quiet | 3 | 30 ms | must be still right after a tap |
| latency | 7 | 560 ms | max gap between the two taps |
Too many false taps → raise the threshold. Missed taps → lower it.

## Power notes
- `sense-edge-mask` on `gpio1`: edge interrupts normally use a GPIOTE channel, which costs
  current in sleep. SENSE detects the level change with almost no current.
- 400 Hz high-performance mode costs the most on the sensor side. Measure it in step 2 and try a
  low-power mode (`power-mode`) or a lower ODR if taps are still detected.

## Lessons from wiring it (2026-11-21)
- Measure the strap pins before guessing the address: SDO/SA0 was low → 0x18.
- **CS selects the bus**: high = I²C, low = SPI. Our breakout left it open, so it had to be tied
  to VDD with a wire. With CS floating the chip may not answer on I²C (`not ready` at boot).

## Check it
- Boot log: `lis2dw12@18: int on gpio@50000300.03` and `accel: lis2dw12@18 ready`.
- `build/app/zephyr/zephyr.dts`: search `lis2dw12@18` to see the final node.
