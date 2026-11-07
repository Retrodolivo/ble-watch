# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# E-ink panel on the Waveshare e-Paper HAT, wired to the DK's Arduino header.
# The shield overlay adds the SSD1681 node; pinout is in docs/hardware.md.
set(SHIELD waveshare_epaper_gdeh0154a07)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_watch)

target_sources(app PRIVATE src/main.c src/clock.c src/ble.c)
