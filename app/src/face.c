/*
 * Binary LED face on top of the Zephyr LED API.
 *
 * Each "pwm-leds" node in the overlay becomes one LED device, and its child
 * nodes are LEDs 0, 1, 2, ... in order. We order the children by bit, so
 * LED index == bit number.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#include "face.h"

LOG_MODULE_REGISTER(face, LOG_LEVEL_INF);

#define HOUR_LEDS_NODE   DT_NODELABEL(hour_leds)
#define MINUTE_LEDS_NODE DT_NODELABEL(minute_leds)

#define HOUR_BITS   DT_CHILD_NUM(HOUR_LEDS_NODE)
#define MINUTE_BITS DT_CHILD_NUM(MINUTE_LEDS_NODE)

/* Wrong number of LEDs in the overlay = build error, not a runtime bug. */
BUILD_ASSERT(HOUR_BITS == 5, "hour_leds needs 5 LEDs (16 8 4 2 1)");
BUILD_ASSERT(MINUTE_BITS == 6, "minute_leds needs 6 LEDs (32 16 8 4 2 1)");

static const struct device *const HourLeds = DEVICE_DT_GET(HOUR_LEDS_NODE);
static const struct device *const MinuteLeds = DEVICE_DT_GET(MINUTE_LEDS_NODE);

/* Set one row of LEDs from a bit pattern. Keeps going on error. */
static int showRow(const struct device *leds, unsigned int count, uint8_t bits) {
  int err = 0;

  for (unsigned int i = 0; i < count; i++) {
    uint8_t level = (bits & BIT(i)) ? CONFIG_FACE_BRIGHTNESS : 0;
    int ret = led_set_brightness(leds, i, level);

    if (ret < 0) {
      LOG_ERR("%s LED %u: %d", leds->name, i, ret);
      err = ret;
    }
  }

  return err;
}

int faceShowBits(uint8_t hour_bits, uint8_t minute_bits) {
  int ret_h = showRow(HourLeds, HOUR_BITS, hour_bits);
  int ret_m = showRow(MinuteLeds, MINUTE_BITS, minute_bits);

  return ret_h < 0 ? ret_h : ret_m;
}

int faceShowTime(uint8_t hours, uint8_t minutes) {
  return faceShowBits(hours % 24, minutes % 60);
}

int faceOff(void) {
  return faceShowBits(0, 0);
}

int faceInit(void) {
  if (!device_is_ready(HourLeds) || !device_is_ready(MinuteLeds)) {
    LOG_ERR("LED devices not ready");
    return -ENODEV;
  }

  LOG_INF("Face ready: %u hour + %u minute LEDs, brightness %d %%", HOUR_BITS, MINUTE_BITS, CONFIG_FACE_BRIGHTNESS);

  return faceOff();
}
