/*
 * ble-watch: step 1, test pattern on the e-ink display, LED blink and logging.
 */

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define BLINK_PERIOD_MS 500

/* Panel size from the shield overlay (width/height properties). */
#define DISPLAY_NODE DT_CHOSEN(zephyr_display)
#define WIDTH        DT_PROP(DISPLAY_NODE, width)
#define HEIGHT       DT_PROP(DISPLAY_NODE, height)
#define SQUARE       25 /* checkerboard square size in pixels */

/* "led0" is an alias in the board devicetree (LED1 on the DK). */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* "zephyr,display" is chosen by the shield overlay. */
static const struct device *const display = DEVICE_DT_GET(DISPLAY_NODE);

/* 1 bit per pixel: 200 * 200 / 8 = 5000 bytes. */
static uint8_t framebuf[WIDTH * HEIGHT / 8];

/*
 * Set one pixel to black. MONO10 means bit 1 = black.
 * The SSD16xx driver uses "vertical tiling" in normal orientation:
 * each byte holds 8 pixels stacked vertically, MSB on top.
 */
static void set_black(int x, int y, bool vtiled)
{
	size_t idx;
	uint8_t bit;

	if (vtiled) {
		idx = (y / 8) * WIDTH + x;
		bit = 7 - (y % 8);
	} else {
		idx = (y * WIDTH + x) / 8;
		bit = 7 - (x % 8);
	}
	framebuf[idx] |= BIT(bit);
}

static int draw_test_pattern(void)
{
	struct display_capabilities caps;
	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(framebuf),
		.width = WIDTH,
		.height = HEIGHT,
		.pitch = WIDTH,
	};
	bool vtiled;
	int ret;

	if (!device_is_ready(display)) {
		LOG_ERR("Display %s not ready", display->name);
		return -ENODEV;
	}

	display_get_capabilities(display, &caps);
	vtiled = (caps.screen_info & SCREEN_INFO_MONO_VTILED) != 0;
	LOG_INF("Display %s: %ux%u, pixel format 0x%x, vtiled %d", display->name,
		caps.x_resolution, caps.y_resolution, caps.current_pixel_format, vtiled);

	/* Checkerboard, plus a solid 3-pixel black border. */
	memset(framebuf, 0, sizeof(framebuf));
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			bool square = ((x / SQUARE) + (y / SQUARE)) % 2 == 0;
			bool border = x < 3 || y < 3 || x >= WIDTH - 3 || y >= HEIGHT - 3;

			if (square || border) {
				set_black(x, y, vtiled);
			}
		}
	}

	ret = display_write(display, 0, 0, &desc, framebuf);
	if (ret < 0) {
		LOG_ERR("display_write failed: %d", ret);
		return ret;
	}

	/* Blanking off triggers the full refresh (takes a few seconds on e-ink). */
	ret = display_blanking_off(display);
	if (ret < 0) {
		LOG_ERR("display_blanking_off failed: %d", ret);
		return ret;
	}

	LOG_INF("Test pattern drawn");
	return 0;
}

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

	/* Keep blinking even if the display fails, so we know the app runs. */
	(void)draw_test_pattern();

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
