/*
 * Battery voltage, measured with the SAADC through a resistor divider.
 */

#ifndef BLE_WATCH_BATTERY_H_
#define BLE_WATCH_BATTERY_H_

#include <stdint.h>

/* Check that the divider and its ADC are ready. */
int batteryInit(void);

/* Measure the battery now. Thread context only (it waits for the ADC). */
int batteryReadMv(int32_t *mv);

#endif /* BLE_WATCH_BATTERY_H_ */
