# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_watch)

target_sources(app PRIVATE 
    src/main.c 
    src/clock.c 
    src/ble.c 
    src/face.c 
    src/accel.c 
    src/battery.c 
    src/heartbeat.c
)
