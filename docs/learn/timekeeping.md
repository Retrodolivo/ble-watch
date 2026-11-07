# Timekeeping on the nRF52840

## Where time comes from
- The nRF52840 has two main clocks: **HFCLK** (64 MHz, for the CPU and radio) and **LFCLK** (32.768 kHz, low power).
- The DK has a 32.768 kHz crystal (LFXO). Kconfig picks it: `CONFIG_CLOCK_CONTROL_NRF_K32SRC_XTAL=y` (default for this board).
- Zephyr's **system timer** on nRF is **RTC1**, running from the LFCLK (`CONFIG_NRF_RTC_TIMER=y`, 32768 ticks/s).
  The RTC keeps counting while the CPU sleeps, so `k_uptime_get()` stays correct in sleep.

## Our clock (app/src/clock.c)
- `time of day = time set + (uptime now - uptime when set)`, modulo 24 h.
- Start value is the build time (`__TIME__`), until Step 3 sets it over BLE.
- No calendar or date: the watch only shows HH:MM.

## Accuracy
- The crystal is rated 50 ppm here (`CONFIG_CLOCK_CONTROL_NRF_K32SRC_50PPM`). 50 ppm = 50 µs per second,
  so up to ~4.3 s per day of drift. Measure the real drift in the 24 h test.

## Timers vs threads
- `k_timer` callback (LED blink) runs in **interrupt context**: keep it short, never sleep there.
- The main thread does `k_msleep(ms to next minute)`: the thread is blocked, the CPU can idle.
