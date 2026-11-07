/*
 * Time of day = (time set by the user) + (uptime since it was set).
 *
 * Uptime comes from the Zephyr system timer. On the nRF52840 that is RTC1,
 * clocked by the 32.768 kHz crystal on the DK (CONFIG_CLOCK_CONTROL_NRF_K32SRC_XTAL).
 * The RTC keeps running in sleep, so no extra hardware timer is needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "clock.h"

LOG_MODULE_REGISTER(clock, LOG_LEVEL_INF);

/* Time of day (in ms since midnight) at the moment clock_set() was called. */
static int64_t base_ms;
/* k_uptime_get() at the moment clock_set() was called. */
static int64_t base_uptime_ms;

static int64_t now_ms(void)
{
	int64_t elapsed = k_uptime_get() - base_uptime_ms;

	return (base_ms + elapsed) % ((int64_t)CLOCK_SECONDS_PER_DAY * MSEC_PER_SEC);
}

void clock_set(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	base_uptime_ms = k_uptime_get();
	base_ms = ((hours % 24) * 3600 + (minutes % 60) * 60 + (seconds % 60)) *
		  (int64_t)MSEC_PER_SEC;

	LOG_INF("Time set to %02u:%02u:%02u", hours % 24, minutes % 60, seconds % 60);
}

uint32_t clock_now(void)
{
	return (uint32_t)(now_ms() / MSEC_PER_SEC);
}

uint32_t clock_ms_to_next_minute(void)
{
	return 60 * MSEC_PER_SEC - (uint32_t)(now_ms() % (60 * MSEC_PER_SEC));
}
