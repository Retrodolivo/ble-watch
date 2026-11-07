# Plan

Each step has a **Build** part (what we make) and a **Learn** part (what you'll understand by the end).

## Step 1: Foundation and "hello display"  ← current
- Build: repo skeleton, toolchain check, LED blink plus logging, then a test image on the e-ink display.
- Learn: NCS, `west` and the build system, devicetree and Kconfig, flashing and logging, SPI, e-ink wiring and controller.
- Done when: the DK shows a test pattern on the display and the pinout is recorded in `docs/hardware.md`.

## Step 2: The clock
- Build: timekeeping with the RTC, HH:MM on screen, partial refresh every minute and a full refresh now and then.
- Learn: nRF52840 clocks and RTC, Zephyr timers and threads, e-ink refresh modes and ghosting, sleep modes, measuring current.
- Done when: it keeps time for 24 h and the idle current has been measured.

## Step 3: Set the time from the phone
- Build: a BLE peripheral with a writable Current Time characteristic (CTS format), written from the nRF Connect app on Android (saved as a macro).
- Learn: advertising, connections, GATT, pairing basics, debugging BLE with the phone.
- Done when: the phone sets the time and the display updates.

## Step 4: Remote firmware updates
- Build: MCUboot plus SMP over BLE, with signed images, updated through the nRF Connect Device Manager app.
- Learn: bootloaders, flash partitions, image signing, swap and rollback, SMP.
- Done when: a new version is installed over the air, and a bad image rolls back.
