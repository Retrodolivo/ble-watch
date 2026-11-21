/*
 * ble-watch: heartbeat LED, binary LED face test at boot, a clock that logs
 * HH:MM at each new minute and is set over BLE (Current Time Service), and
 * a double-tap on the accelerometer that shows the time for a few seconds.
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "accel.h"
#include "ble.h"
#include "clock.h"
#include "face.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define BLINK_PERIOD_MS 500
#define WALK_STEP_MS    200  /* per LED in the boot test */
#define SHOW_TIME_MS    5000 /* how long the time stays on */

/* "led0" is an alias in the board devicetree (LED1 on the DK). */
static const struct gpio_dt_spec Led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Runs in interrupt context every BLINK_PERIOD_MS: keep it short. */
static void blinkHandler(struct k_timer *timer) {
  ARG_UNUSED(timer);
  gpio_pin_toggle_dt(&Led);
}

K_TIMER_DEFINE(blink_timer, blinkHandler, NULL);

/*
 * Boot test pattern: light each LED alone, hours 1..16 then minutes 1..32,
 * so a wrong or dead wire is easy to spot. Then show the time, then off.
 * k_msleep() puts the thread to sleep: the CPU is idle in between.
 */
static void faceTest(void) {
  for (int i = 0; i < 5; i++) {
    (void)faceShowBits(BIT(i), 0);
    k_msleep(WALK_STEP_MS);
  }
  for (int i = 0; i < 6; i++) {
    (void)faceShowBits(0, BIT(i));
    k_msleep(WALK_STEP_MS);
  }

  uint32_t now = clockNow();

  LOG_INF("Face shows %02u:%02u", now / 3600, (now / 60) % 60);
  (void)faceShowTime(now / 3600, (now / 60) % 60);
  k_msleep(SHOW_TIME_MS);
  (void)faceOff();
}

static void faceOffHandler(struct k_work *work) {
  ARG_UNUSED(work);
  (void)faceOff();
}

K_WORK_DELAYABLE_DEFINE(face_off_work, faceOffHandler);

/*
 * Double-tap: show the time, and turn the LEDs off SHOW_TIME_MS later.
 * Runs in the system work queue, like faceOffHandler(), so the two never
 * run at the same time. A new double-tap while lit restarts the countdown.
 */
static void onDoubleTap(void) {
  uint32_t now = clockNow();

  LOG_INF("Face shows %02u:%02u", now / 3600, (now / 60) % 60);
  (void)faceShowTime(now / 3600, (now / 60) % 60);
  k_work_reschedule(&face_off_work, K_MSEC(SHOW_TIME_MS));
}

/* Start time = build time ("HH:MM:SS" from the compiler), until BLE can set it. */
static void clockSetFromBuildTime(void) {
  unsigned int h, m, s;

  if (sscanf(__TIME__, "%u:%u:%u", &h, &m, &s) == 3) {
    clockSet(h, m, s);
  } else {
    clockSet(0, 0, 0);
  }
}

int main(void) {
  int ret;

  LOG_INF("ble-watch starting on %s", CONFIG_BOARD_TARGET);

  if (!gpio_is_ready_dt(&Led)) {
    LOG_ERR("LED GPIO device not ready");
    return 0;
  }

  ret = gpio_pin_configure_dt(&Led, GPIO_OUTPUT_INACTIVE);
  if (ret < 0) {
    LOG_ERR("Failed to configure LED: %d", ret);
    return 0;
  }

  /* Heartbeat: shows that the app runs. */
  k_timer_start(&blink_timer, K_MSEC(BLINK_PERIOD_MS), K_MSEC(BLINK_PERIOD_MS));

  clockSetFromBuildTime();

  if (faceInit() == 0) {
    faceTest();
  }

  /* If the accelerometer is missing, the rest still runs. */
  (void)accelInit(onDoubleTap);

  /* If BLE fails, the clock still runs from the build time. */
  (void)bleInit();

  while (1) {
    /*
     * Sleep until hh:mm:00, or until the time is set (at boot or
     * from the phone). The thread uses no CPU while it waits.
     */
    (void)clockWaitNextMinute();

    uint32_t now = clockNow();

    LOG_INF("Time %02u:%02u (uptime %lld s)", now / 3600, (now / 60) % 60, k_uptime_get() / MSEC_PER_SEC);
  }

  return 0;
}
