#include "ble.h"

static const char *tag = "BLE_MODULE";

// BLE Spam
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = BLE_ADV_INT_MIN,
    .adv_int_max        = BLE_ADV_INT_MAX,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_gap_ble_cb_param_t *param) {
	switch (event) {

		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:

			ESP_LOGI(tag, "Advertising data set successfully.");
			esp_err_t ret = esp_ble_gap_start_advertising(&adv_params);
			if (ret != ESP_OK) {
				ESP_LOGE(tag, "Failed to start advertising: %s", esp_err_to_name(ret));
			} else {
				ESP_LOGI(tag, "Advertising started...");
			}
			break;

		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			ESP_LOGI(tag, "Advertising start complete.");
			break;

		default:
			break;
	}
}
void ble_init(void) {
	esp_err_t ret;

	ESP_LOGI(tag, "Initializing Bluetooth Controller...");
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);

	if (ret) {
		ESP_LOGE(tag, "Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
		return;
	}

	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(tag, "Bluetooth controller enabling failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(tag, "Initializing Bluedroid");
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(tag, "Bluedroid initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(tag, "Bluedroid enabling failed: %s", esp_err_to_name(ret));
        return;
    }

	esp_ble_gap_register_callback(ble_gap_event_handler);
}
static void ble_advertise(void) {
	esp_ble_gap_config_adv_data_t adv_data = {
		.set_scan_rsp = false,
		.include_name = true,
		.include_txpower = false,
		.min_interval = BLE_ADV_INT_MIN,
		.max_interval = BLE_ADV_INT_MAX,
		.appearance = 0x00,
		.manufacturer_len = 0,
		.p_manufacturer_data = NULL,
		.service_data_len = 0,
		.p_service_data = NULL,
		.service_uuid_len = 0,
		.p_service_uuid = NULL,
		.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT
	};

	esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret != ESP_OK) {
        ESP_LOGE(tag, "Failed to configure advertising data: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(tag, "Advertising data configured.");
    }
}