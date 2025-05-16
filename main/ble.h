#pragma once

// ble spam
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_bt_defs.h"

// keystroke injection
#include "esp_gatts_api.h"
#include "esp_hidd_api.h"
#include "esp_hidd.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"

#define BLE_ADV_INT_MIN 0x20
#define BLE_ADV_INT_MAX 0x30
#define BLE_SPAM_DEVICE_NAME "macero"

void ble_init(void);
void ble_advertise(void);