/*
 * Heartbeat LED on top of the Zephyr LED API ("gpio-leds" node status_led).
 *
 * Two ways to light it, both in the battery colour (green OK, red low):
 *
 * 1. Periodic flash. One delayable work item, one step per run:
 *      off -> measure the battery, LED on, wait CONFIG_HEARTBEAT_FLASH_MS
 *      on  -> LED off, wait the rest of CONFIG_HEARTBEAT_PERIOD_S
 *
 * 2. With the face (heartbeatShow() / heartbeatHide(), called on a
 *    double-tap). The periodic flash stops, the LED stays on while the face
 *    is lit, and the next periodic flash comes a full period after
 *    heartbeatHide().
 *
 * The battery is always measured with the face off: before the face lights
 * up, and the periodic flash never runs while the face is lit. So the 11
 * face LEDs' current can't pull the reading down.
 *
 * Red below CONFIG_BATTERY_LOW_MV; back to green only at
 * CONFIG_BATTERY_LOW_MV + CONFIG_BATTERY_HYSTERESIS_MV. If a reading fails,
 * the error is logged and the last colour is kept.
 *
 * Everything runs in the system work queue (the double-tap handler and
 * face_off_work too), one item at a time, so the state needs no lock.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#include "battery.h"
#include "heartbeat.h"

LOG_MODULE_REGISTER(heartbeat, LOG_LEVEL_INF);

#define STATUS_LED_NODE DT_NODELABEL(status_led)

#define PERIOD_MS (CONFIG_HEARTBEAT_PERIOD_S * MSEC_PER_SEC)

BUILD_ASSERT(DT_CHILD_NUM(STATUS_LED_NODE) == 2, "status_led needs 2 LEDs (red, green)");
BUILD_ASSERT(CONFIG_HEARTBEAT_FLASH_MS < PERIOD_MS, "flash longer than period");

/* LED index = child order in the overlay. A colour is a mask of these bits. */
#define COLOR_RED   BIT(0)
#define COLOR_GREEN BIT(1)
#define COLOR_OFF   0

static const struct device *const StatusLed = DEVICE_DT_GET(STATUS_LED_NODE);

static bool ready;      /* heartbeatInit() succeeded */
static bool lit;        /* the LED is on (periodic flash or with the face) */
static bool held;       /* on with the face, between Show and Hide */
static bool batteryLow; /* last result, with hysteresis */

static void setColor(uint8_t color) {
  for (uint32_t i = 0; i < DT_CHILD_NUM(STATUS_LED_NODE); i++) {
    int ret = (color & BIT(i)) ? led_on(StatusLed, i) : led_off(StatusLed, i);

    if (ret < 0) {
      LOG_ERR("Status LED %u: %d", i, ret);
    }
  }
}

/* Measure the battery and update batteryLow. */
static void measureBattery(void) {
  int32_t mv;

  if (batteryReadMv(&mv) < 0) {
    return;
  }

  LOG_INF("Battery %d mV", mv);

  /*
   * Hysteresis: two thresholds instead of one. A battery at 3.60 V reads
   * 3.598 V one minute and 3.602 V the next; with one threshold the LED
   * would switch colour every minute.
   */
  if (batteryLow) {
    batteryLow = mv < CONFIG_BATTERY_LOW_MV + CONFIG_BATTERY_HYSTERESIS_MV;
  } else {
    batteryLow = mv < CONFIG_BATTERY_LOW_MV;
  }
}

static uint8_t batteryColor(void) {
  return batteryLow ? COLOR_RED : COLOR_GREEN;
}

static void heartbeatHandler(struct k_work *work) {
  struct k_work_delayable *dwork = k_work_delayable_from_work(work);

  if (lit) {
    setColor(COLOR_OFF);
    lit = false;
    k_work_schedule(dwork, K_MSEC(PERIOD_MS - CONFIG_HEARTBEAT_FLASH_MS));
  } else {
    measureBattery();
    setColor(batteryColor());
    lit = true;
    k_work_schedule(dwork, K_MSEC(CONFIG_HEARTBEAT_FLASH_MS));
  }
}

static K_WORK_DELAYABLE_DEFINE(heartbeat_work, heartbeatHandler);

void heartbeatShow(void) {
  if (!ready) {
    return;
  }

  /* Stop the periodic flash. Same work queue, so it isn't running now. */
  k_work_cancel_delayable(&heartbeat_work);

  if (held) {
    return; /* face already lit: don't measure under its load */
  }

  /* During a periodic flash the battery was measured just now. */
  if (!lit) {
    measureBattery();
  }

  held = true;
  lit = true;
  setColor(batteryColor());
}

void heartbeatHide(void) {
  if (!ready || !held) {
    return;
  }

  setColor(COLOR_OFF);
  held = false;
  lit = false;

  /* Next periodic flash a full period from now. */
  k_work_schedule(&heartbeat_work, K_MSEC(PERIOD_MS));
}

int heartbeatInit(void) {
  if (!device_is_ready(StatusLed)) {
    LOG_ERR("Status LED not ready");
    return -ENODEV;
  }

  batteryInit();

  setColor(COLOR_OFF);
  ready = true;
  k_work_schedule(&heartbeat_work, K_NO_WAIT);

  LOG_INF("Heartbeat every %d s", CONFIG_HEARTBEAT_PERIOD_S);

  return 0;
}
