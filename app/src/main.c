/*
 * ble-watch: binary LED face test at boot, a clock that logs HH:MM at each
 * new minute and is set over BLE (Current Time Service), a double-tap on the
 * accelerometer that shows the time for a few seconds, and red/green heartbeat
 * LEDs that flash once a minute to show the battery level.
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "accel.h"
#include "ble.h"
#include "clock.h"
#include "face.h"
#include "heartbeat.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define WALK_STEP_MS 200  /* per LED in the boot test */
#define SHOW_TIME_MS 5000 /* how long the time stays on */

/*
 * Boot test pattern: light each LED alone, hours 1..16 then minutes 1..32.
 * Then show the time, then off.
 */
static void faceTest(void) {
  for (int i = 0; i < 5; i++) {
    faceShowBits(BIT(i), 0);
    k_msleep(WALK_STEP_MS);
  }
  for (int i = 0; i < 6; i++) {
    faceShowBits(0, BIT(i));
    k_msleep(WALK_STEP_MS);
  }

  uint32_t now = clockNow();

  LOG_INF("Face shows %02u:%02u", now / 3600, (now / 60) % 60);
  faceShowTime(now / 3600, (now / 60) % 60);
  k_msleep(SHOW_TIME_MS);
  faceOff();
}

static void faceOffHandler(struct k_work *work) {
  ARG_UNUSED(work);
  faceOff();
  heartbeatHide();
}

K_WORK_DELAYABLE_DEFINE(face_off_work, faceOffHandler);

/*
 * Double-tap: show the time and the heartbeat LED.
 */
static void onDoubleTap(void) {
  uint32_t now = clockNow();

  heartbeatShow();

  LOG_INF("Face shows %02u:%02u", now / 3600, (now / 60) % 60);
  faceShowTime(now / 3600, (now / 60) % 60);
  k_work_reschedule(&face_off_work, K_MSEC(SHOW_TIME_MS));
}

/* Start time = build time ("HH:MM:SS" from the compiler), until the phone sets it over BLE. */
static void clockSetFromBuildTime(void) {
  unsigned int h, m, s;

  if (sscanf(__TIME__, "%u:%u:%u", &h, &m, &s) == 3) {
    clockSet(h, m, s);
  } else {
    clockSet(0, 0, 0);
  }
}

int main(void) {
  LOG_INF("ble-watch starting on %s", CONFIG_BOARD_TARGET);

  clockSetFromBuildTime();

  if (faceInit() == 0) {
    faceTest();
  }

  /* Heartbeat: shows that the app runs, and the battery level. */
  heartbeatInit();

  /* If the accelerometer is missing, the rest still runs. */
  accelInit(onDoubleTap);

  /* If BLE fails, the clock still runs from the build time. */
  bleInit();

  while (1) {
    /*
     * Sleep until hh:mm:00, or until the time is set (at boot or
     * from the phone). The thread uses no CPU while it waits.
     */
    clockWaitNextMinute();

    uint32_t now = clockNow();

    LOG_INF("Time %02u:%02u (uptime %lld s)", now / 3600, (now / 60) % 60, k_uptime_get() / MSEC_PER_SEC);
  }

  return 0;
}
