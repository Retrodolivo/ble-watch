# Blinky and logging (step 1, first task)

## The app files
- `app/CMakeLists.txt`: `find_package(Zephyr)` pulls in the whole Zephyr build system; `target_sources(app ...)` adds our `.c` files to the `app` library.
- `app/prj.conf`: our Kconfig choices. They are merged with the board's defaults (`nrf52840dk_nrf52840_defconfig`). The result is `build/app/zephyr/.config`.
- `app/src/main.c`: `main()` runs in Zephyr's main thread after the kernel and drivers are initialized.

## LED from devicetree
- Board DTS: `led0: led_0 { gpios = <&gpio0 13 GPIO_ACTIVE_LOW>; }` and the alias `led0 = &led0;`.
- `GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)` is resolved **at compile time** into `{ port device, pin 13, flags ACTIVE_LOW }`. No pin numbers in our code.
- `GPIO_ACTIVE_LOW`: the DK's LEDs light up when the pin is low. With the `_dt` API, "active" means "LED on", and Zephyr handles the inversion.
- `gpio_is_ready_dt()`: checks that the GPIO driver finished init. Always check before use.

## Logging
- `CONFIG_LOG=y` turns on the logging subsystem. `LOG_MODULE_REGISTER(main, LOG_LEVEL_INF)` once per file names the module.
- Levels: ERR=1, WRN=2, INF=3, DBG=4. Messages above the module's level are removed at compile time (cost nothing).
- **Deferred mode** (`CONFIG_LOG_MODE_DEFERRED=y`, default): `LOG_INF()` only puts the message in a buffer; a low-priority log thread prints it later. Fast, but the last messages can be lost on a crash.
- Backend: UART (`CONFIG_LOG_BACKEND_UART=y`) on the `zephyr,console` chosen node = `uart0`, which the DK routes to the J-Link's VCOM0.

## Sleeping
- `k_msleep()` blocks the thread; the scheduler runs other threads or idles the CPU. Never busy-wait.
