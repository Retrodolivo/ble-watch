/*
 * BLE peripheral: advertises as CONFIG_BT_DEVICE_NAME and exposes the
 * Current Time Service (CTS, UUID 0x1805) from Zephyr (CONFIG_BT_CTS).
 * Writing the Current Time characteristic (0x2A2B) sets our clock.
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/services/cts.h>
#include <zephyr/logging/log.h>

#include "ble.h"
#include "clock.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

/* Advertising packet: flags + "I have a Current Time Service". */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)),
};

/* Scan response: the device name, sent when the phone asks for more. */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* Local Time Information (time zone, DST): stored, but not used for HH:MM. */
static struct bt_cts_local_time local_time = {
	.timezone_offset = BT_CTS_TIMEZONE_DEFAULT_VALUE,
	.dst_offset = BT_CTS_DST_OFFSET_UNKNOWN,
};

static void adv_start(void)
{
	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (err) {
		LOG_ERR("Advertising failed to start: %d", err);
		return;
	}
	LOG_INF("Advertising as \"%s\"", CONFIG_BT_DEVICE_NAME);
}

/* Restart advertising from a work item, not from inside the BT callback. */
static void adv_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	adv_start();
}

static K_WORK_DEFINE(adv_work, adv_work_handler);

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	if (err) {
		LOG_WRN("Connection to %s failed: 0x%02x", addr, err);
		return;
	}
	LOG_INF("Connected: %s", addr);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected, reason 0x%02x", reason);
}

/* The connection object is free again: now we can advertise for a new one. */
static void recycled(void)
{
	k_work_submit(&adv_work);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.recycled = recycled,
};

/* Phone wrote Current Time: 10 bytes, we only use hours, minutes and seconds. */
static int cts_time_write(struct bt_cts_time_format *t)
{
	if (t->hours > 23 || t->min > 59 || t->sec > 59) {
		LOG_WRN("Rejected time %u:%u:%u", t->hours, t->min, t->sec);
		return -EINVAL;
	}

	clock_set(t->hours, t->min, t->sec);
	return 0;
}

/* Phone reads Current Time. We don't track the date: 0 means "unknown" in CTS. */
static int cts_fill_time(struct bt_cts_time_format *t)
{
	uint32_t now = clock_now();

	*t = (struct bt_cts_time_format){
		.hours = now / 3600,
		.min = (now / 60) % 60,
		.sec = now % 60,
	};
	return 0;
}

static int cts_local_time_write(const struct bt_cts_local_time *lt)
{
	local_time = *lt;
	return 0;
}

static int cts_fill_local_time(struct bt_cts_local_time *lt)
{
	*lt = local_time;
	return 0;
}

static const struct bt_cts_cb cts_cb = {
	.cts_time_write = cts_time_write,
	.fill_current_cts_time = cts_fill_time,
	.cts_local_time_write = cts_local_time_write,
	.fill_current_cts_local_time = cts_fill_local_time,
};

int ble_init(void)
{
	int err;

	err = bt_cts_init(&cts_cb);
	if (err) {
		LOG_ERR("CTS init failed: %d", err);
		return err;
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed: %d", err);
		return err;
	}

	adv_start();
	return 0;
}
