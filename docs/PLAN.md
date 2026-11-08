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
- Progress (started while waiting for the display HAT): clock module that logs HH:MM each minute (`app/src/clock.c`). Flashed: logs the time each minute. 24 h drift test still to do.

## Step 3: Set the time from the phone
- Build: a BLE peripheral with a writable Current Time characteristic (CTS format), written from the nRF Connect app on Android (saved as a macro).
- Learn: advertising, connections, GATT, pairing basics, debugging BLE with the phone.
- Done when: the phone sets the time and the display updates.
- Progress: advertising as "ble-watch" with Zephyr's CTS; writing Current Time sets the clock (`app/src/ble.c`). Flashed and tested with nRF Connect. Display update waits for the HAT.

## Step 4: Remote firmware updates
- Build: MCUboot plus SMP over BLE, with signed images, updated through the nRF Connect Device Manager app.
- Learn: bootloaders, flash partitions, image signing, swap and rollback, SMP.
- Done when: a new version is installed over the air, and a bad image rolls back.
- Progress (started while waiting for the display HAT): MCUboot added via sysbuild (`app/sysbuild.conf`), app signed with the demo key. Flashed: MCUboot boots the app, BLE still works. SMP over BLE added (`CONFIG_NCS_SAMPLE_MCUMGR_BT_OTA_DFU`): 0.1.0 -> 0.2.0 installed over the air with Device Manager. Next: rollback test.
