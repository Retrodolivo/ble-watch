/*
 * ble-watch: step 1, LED blink and logging.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define BLINK_PERIOD_MS 500

/* "led0" is an alias in the board devicetree (LED1 on the DK). */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	uint32_t count = 0;
	int ret;

	LOG_INF("ble-watch starting on %s", CONFIG_BOARD_TARGET);

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED: %d", ret);
		return 0;
	}

	while (1) {
		gpio_pin_toggle_dt(&led);
		if ((count % 10) == 0) {
			LOG_INF("alive, toggle #%u", count);
		}
		count++;
		k_msleep(BLINK_PERIOD_MS);
	}

	return 0;
}
