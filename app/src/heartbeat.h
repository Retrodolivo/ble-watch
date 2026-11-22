/*
 * Heartbeat: a red and a green status LED that flash once a minute, showing
 * the battery level (see the overlay and Kconfig for the settings).
 * Call these from the system work queue only (like the double-tap handler).
 */

#ifndef BLE_WATCH_HEARTBEAT_H_
#define BLE_WATCH_HEARTBEAT_H_

/* Check the LED and the battery, then start flashing (first flash now). */
int heartbeatInit(void);

/*
 * The face is about to light up: measure the battery (face still off), then
 * keep the LED on in the battery colour until heartbeatHide().
 */
void heartbeatShow(void);

/* The face is off: LED off, next periodic flash a full period from now. */
void heartbeatHide(void);

#endif /* BLE_WATCH_HEARTBEAT_H_ */
