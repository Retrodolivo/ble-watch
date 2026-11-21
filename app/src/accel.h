/*
 * LIS2DW12 accelerometer: double-tap detection (see the overlay for settings).
 */

#ifndef BLE_WATCH_ACCEL_H_
#define BLE_WATCH_ACCEL_H_

/* Called on each double-tap, from the system work queue (thread context). */
typedef void (*accel_double_tap_cb_t)(void);

/* Check the sensor is ready and start double-tap detection. */
int accel_init(accel_double_tap_cb_t cb);

#endif /* BLE_WATCH_ACCEL_H_ */
