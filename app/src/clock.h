/*
 * Wall-clock time of day, counted from the Zephyr system timer.
 */

#ifndef BLE_WATCH_CLOCK_H_
#define BLE_WATCH_CLOCK_H_

#include <stdint.h>

#define CLOCK_SECONDS_PER_DAY (24 * 60 * 60)

/* Set the current time of day. */
void clock_set(uint8_t hours, uint8_t minutes, uint8_t seconds);

/* Seconds since midnight, 0 .. CLOCK_SECONDS_PER_DAY - 1. */
uint32_t clock_now(void);

/* Milliseconds until the next full minute (00 seconds). */
uint32_t clock_ms_to_next_minute(void);

#endif /* BLE_WATCH_CLOCK_H_ */
