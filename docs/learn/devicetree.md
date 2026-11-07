# Devicetree: from `.dts` to C

How `GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)` in `app/src/main.c` gets its values.

## 1. Sources (merged in this order, later wins)
- SoC: `zephyr/dts/arm/nordic/nrf52840.dtsi` defines `gpio0: gpio@50000000` (the hardware block).
- Board: `zephyr/boards/nordic/nrf52840dk/nrf52840dk_nrf52840.dts` defines `led0: led_0 { gpios = <&gpio0 13 GPIO_ACTIVE_LOW>; }` and the alias `led0 = &led0;`.
- App overlay: `app/boards/nrf52840dk_nrf52840.overlay` (none yet; the display goes here).

## 2. Build output
- `build/app/zephyr/zephyr.dts`: the merged tree. Each line has a comment saying which file set it. Best place to debug.
- `build/app/zephyr/include/generated/zephyr/devicetree_generated.h`: the same tree as C macros.

## 3. Macro chain (all at compile time)
```
DT_ALIAS(led0)            -> DT_N_ALIAS_led0 -> DT_N_S_leds_S_led_0     (node "/leds/led_0")
  gpios[0] controller     -> DT_N_S_soc_S_gpio_50000000                 (gpio0)
  gpios[0] pin            -> 13
  gpios[0] flags          -> 1  (GPIO_ACTIVE_LOW)
DEVICE_DT_GET(gpio0)      -> &__device_dts_ord_11                        (driver's struct device)
```
`GPIO_DT_SPEC_GET` (in `zephyr/drivers/gpio.h`) builds `{ .port, .pin, .dt_flags }` from these.

## 4. Who creates `__device_dts_ord_11`?
- The node has `compatible = "nordic,nrf-gpio"` and `status = "okay"`.
- `drivers/gpio/gpio_nrfx.c` has `#define DT_DRV_COMPAT nordic_nrf_gpio` and creates one `struct device` per matching node.
- A **binding** (`dts/bindings/.../nordic,nrf-gpio.yaml`) says which properties the node may have.
- Kconfig `CONFIG_GPIO=y` decides if the driver is compiled at all. Devicetree says *what hardware exists*; Kconfig says *what software is built*.

## Takeaways
- Wrong node name or alias = **build error**, not a runtime bug.
- `status = "disabled"` = no device struct = `gpio_is_ready_dt()` would fail / link error.
