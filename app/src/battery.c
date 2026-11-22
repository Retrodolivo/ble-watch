/*
 * Battery voltage with Zephyr's "voltage-divider" sensor driver.
 *
 * The driver owns the ADC channel from the overlay. sensor_sample_fetch()
 * runs one SAADC conversion; sensor_channel_get() converts it to volts and
 * scales it by the divider ratio (full-ohms / output-ohms), so we get VBAT.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "battery.h"

LOG_MODULE_REGISTER(battery, LOG_LEVEL_INF);

static const struct device *const Divider = DEVICE_DT_GET(DT_NODELABEL(vbatt));

int batteryInit(void) {
  if (!device_is_ready(Divider)) {
    LOG_ERR("%s not ready", Divider->name);
    return -ENODEV;
  }

  return 0;
}

int batteryReadMv(int32_t *mv) {
  struct sensor_value val;
  int ret;

  /* Its init failed (e.g. bad ADC channel config): don't call the driver. */
  if (!device_is_ready(Divider)) {
    return -ENODEV;
  }

  ret = sensor_sample_fetch_chan(Divider, SENSOR_CHAN_VOLTAGE);
  if (ret < 0) {
    LOG_ERR("Battery fetch: %d", ret);
    return ret;
  }

  ret = sensor_channel_get(Divider, SENSOR_CHAN_VOLTAGE, &val);
  if (ret < 0) {
    LOG_ERR("Battery get: %d", ret);
    return ret;
  }

  *mv = (int32_t)sensor_value_to_milli(&val);

  return 0;
}
