# Zephyr display API and the e-ink test pattern (step 1)

## How the display gets into the build
- `app/CMakeLists.txt`: `set(SHIELD waveshare_epaper_gdeh0154a07)` before `find_package(Zephyr)`.
  Same as `-DSHIELD=...` on the command line, but you can't forget it.
- The shield overlay adds a `zephyr,mipi-dbi-spi` node (the SPI "bus adapter" with DC and RST pins)
  and under it the panel node `solomon,ssd1681` (with BUSY, width, height).
- `prj.conf`: only `CONFIG_DISPLAY=y`. `CONFIG_SSD16XX`, `CONFIG_MIPI_DBI_SPI`, `CONFIG_SPI` turn on by
  themselves because enabled devicetree nodes exist. Check in `build/app/zephyr/.config`.

## Layers
```
main.c  display_write()            generic API, same for any display
ssd16xx driver                     SSD1681 commands, LUTs, refresh
mipi_dbi_spi                       "command or data?" via the DC pin, reset pin
spi_nrfx_spim                      the nRF52840 SPIM3 hardware
```

## Framebuffer format
- `PIXEL_FORMAT_MONO10`: 1 bit per pixel, **1 = black**, 0 = white. 200x200 -> 5000 bytes.
- `SCREEN_INFO_MONO_VTILED`: each byte is 8 pixels **stacked vertically** (a "page"), MSB on top.
  Byte index = `(y / 8) * width + x`. Without VTILED, bytes run horizontally.
- `display_get_capabilities()` tells you both at runtime; `set_black()` in `main.c` handles both.

## E-ink behaviour
- After init the driver has **blanking on**: `display_write()` only fills the controller's RAM.
- `display_blanking_off()` runs a **full refresh**: the panel flashes black/white a few times
  (that's normal, it removes ghosting) and then shows the image. BUSY is high while it works.
- With blanking off, later writes use **partial refresh** (fast, no flashing). That's what the clock will use.
- The image stays after power-off: e-ink needs power only to change.
