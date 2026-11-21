/*
 * Double-tap detection with the Zephyr sensor API.
 *
 * The LIS2DW12 detects the double-tap by itself and raises INT1. The driver
 * catches the GPIO interrupt, reads the sensor's interrupt sources over I2C
 * in the system work queue, and calls our trigger handler. The CPU does
 * nothing in between: no polling.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "accel.h"

LOG_MODULE_REGISTER(accel, LOG_LEVEL_INF);

static const struct device *const accel = DEVICE_DT_GET(DT_NODELABEL(accel));

static accel_double_tap_cb_t double_tap_cb;

/* Must stay valid after sensor_trigger_set(): the driver keeps a pointer. */
static const struct sensor_trigger double_tap_trig = {
	.type = SENSOR_TRIG_DOUBLE_TAP,
	.chan = SENSOR_CHAN_ACCEL_XYZ,
};

static void double_tap_handler(const struct device *dev, const struct sensor_trigger *trig)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(trig);

	LOG_INF("Double-tap");

	if (double_tap_cb != NULL) {
		double_tap_cb();
	}
}

int accel_init(accel_double_tap_cb_t cb)
{
	int ret;

	/* False if the driver's init failed, e.g. no answer on I2C (wiring, address). */
	if (!device_is_ready(accel)) {
		LOG_ERR("%s not ready (check wiring and I2C address)", accel->name);
		return -ENODEV;
	}

	double_tap_cb = cb;

	ret = sensor_trigger_set(accel, &double_tap_trig, double_tap_handler);
	if (ret < 0) {
		LOG_ERR("Double-tap trigger: %d", ret);
		return ret;
	}

	LOG_INF("%s ready, waiting for a double-tap", accel->name);

	return 0;
}
