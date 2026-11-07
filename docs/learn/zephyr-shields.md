# Zephyr shields and how pins get resolved

- A **shield** is a devicetree overlay (+ optional Kconfig) for an add-on board. Enable it at build time:
  `west build -b nrf52840dk/nrf52840 <app> -- -DSHIELD=waveshare_epaper_gdeh0154a07`
- The shield overlay never names nRF pins. It says `&arduino_header ARDUINO_HEADER_R3_D9`.
  The **board** devicetree has a `gpio-map` that turns "Arduino D9" into a real pin (P1.11 on the DK).
  So the same shield works on any board with an Arduino header.
- `spi-dev = <&arduino_spi>` works the same way: the board says `arduino_spi` is `spi3` (SCK P1.15, MOSI P1.13, CS P1.12).
- Kconfig follows devicetree: a node with `compatible = "solomon,ssd1681"` and `status = "okay"` makes
  `CONFIG_SSD16XX` default to `y`. The app only needs `CONFIG_DISPLAY=y`.
- `chosen { zephyr,display = ...; }` lets app code get the display with `DEVICE_DT_GET(DT_CHOSEN(zephyr_display))`
  without knowing which driver it is.
- To see the final result, open `build/<app>/zephyr/zephyr.dts` (merged devicetree, with a comment showing which file set each line)
  and `build/<app>/zephyr/.config` (final Kconfig).
- With sysbuild (the NCS default) the app's build output is in `build/<app-name>/`, not directly in `build/`.
