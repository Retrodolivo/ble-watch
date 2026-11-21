/*
 * Binary LED face: 5 hour LEDs and 6 minute LEDs (PWM LEDs, see the overlay).
 */

#ifndef BLE_WATCH_FACE_H_
#define BLE_WATCH_FACE_H_

#include <stdint.h>

/* Check that the LED devices are ready. Leaves all LEDs off. */
int faceInit(void);

/* Light the raw bit patterns: bit 0 of each byte is the "1" LED. */
int faceShowBits(uint8_t hour_bits, uint8_t minute_bits);

/* Show a time of day in binary (hours 0-23, minutes 0-59). */
int faceShowTime(uint8_t hours, uint8_t minutes);

/* All LEDs off. With every channel at 0 %, the PWM peripherals stop. */
int faceOff(void);

#endif /* BLE_WATCH_FACE_H_ */
