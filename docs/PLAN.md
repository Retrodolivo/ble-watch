# Plan

Each step has a **Build** part (what we make) and a **Learn** part (what you'll understand by the end).
Steps 1–4 run on the nRF52840 DK. Step 5 moves the firmware to the real watch.
Steps 1b and 4b add CI/CD (build and release automation on GitHub) and don't change the firmware itself.

## Step 1: Foundation, LED face and accelerometer  ← current
- Build: repo skeleton, toolchain check, logging, then the 11 binary LEDs on a breadboard (a counter or test pattern), then the LIS2DW12 breakout with a double-tap interrupt that lights the LEDs.
- Learn: NCS, `west` and the build system, devicetree and Kconfig, GPIO and the LED API, flashing and logging, I²C, sensor drivers and interrupts (triggers).
- Done when: a double-tap on the accelerometer lights a test pattern on the LEDs, and the pinout is recorded in `docs/hardware.md`.

## Step 1b: Continuous integration (CI) on a self-hosted runner
- Build: a GitHub Actions workflow (`.github/workflows/build.yml`) that builds the firmware with sysbuild on every push to `main`. It runs on a **self-hosted GitHub runner on the VPS**, not on GitHub's machines. The runner runs as a service under its own unprivileged user. The VPS keeps a pinned NCS v3.4.2 workspace (or a Docker image with it) between runs, so each build doesn't download several GB again. Each run saves the firmware files (`merged.hex`, `zephyr.signed.bin`) as downloadable artifacts.
- Security: the repo is public, and a self-hosted runner runs whatever code the workflow is given. So the workflow does not run for pull requests from forks, and the repo setting "Require approval for all external contributors" is on.
- Learn: GitHub Actions (workflows, jobs, steps, runners, artifacts), installing a runner as a service, reproducible builds with a pinned toolchain, caching, why self-hosted runners on public repos are risky.
- Done when: a push to GitHub starts a build on the VPS, the commit gets a green check (or a red cross when the build breaks), and the firmware can be downloaded from the run page.

## Step 2: The clock
- Build: timekeeping with the RTC, the time shown in binary (24-hour) for a few seconds after a double-tap, then the LEDs turn off and the chip goes back to sleep.
- Learn: nRF52840 clocks and RTC, Zephyr timers, work queues and threads, sleep and power management, measuring current (e.g. with a Power Profiler Kit).
- Done when: it keeps time for 24 h and the idle current has been measured.

## Step 3: Set the time from the phone
- Build: a BLE peripheral with a writable Current Time characteristic (CTS format), written from the nRF Connect app on Android (saved as a macro).
- Learn: advertising, connections, GATT, pairing basics, debugging BLE with the phone.
- Done when: the phone sets the time and the next double-tap shows it.

## Step 4: Remote firmware updates
- Build: MCUboot plus SMP over BLE, with signed images, updated through the nRF Connect Device Manager app.
- Learn: bootloaders, flash partitions, image signing, swap and rollback, SMP.
- Done when: a new version is installed over the air, and a bad image rolls back.

## Step 4b: Continuous delivery (CD): releases from a tag
- Build: a second workflow, on the same VPS runner, that starts when a version tag is pushed (e.g. `0.3.0`, the same style as the existing `0.0.1` tag). It checks that the tag matches the app's version, builds and signs the image, and publishes a **GitHub Release** with the OTA update file (`dfu_application.zip`) and the full `merged.hex` for SWD flashing.
- Signing: replace MCUboot's default (public, insecure) key with our own private key. The key lives only in a GitHub secret and an offline backup, never in the repo. The bootloader has to be reflashed over SWD once, so that it trusts the new key.
- Installing stays manual: the VPS can't reach the watch over BLE, so the phone downloads the release file and uploads it with the nRF Connect Device Manager app.
- Learn: release flow, version numbers (git tag vs MCUboot image version), signing keys and GitHub secrets, why the bootloader's key and the image's key must match.
- Done when: pushing a tag creates a release, and the release file installs over BLE with Device Manager.

## Step 5: The real watch (later)
- Build: a custom PCB (bare nRF52840, nPM1100, LIS2DW12, 11 LEDs, chip antenna, from Nordic's reference layout, assembled by JLCPCB/PCBWay), a board definition for Zephyr, and the 3D-printed case. First flash over SWD pads, then OTA only.
- Learn: PCB design, RF matching and antenna tuning, custom Zephyr boards, battery charging and protection.
- Done when: the watch runs on its battery, charges from the pogo pads, and updates over the air.
