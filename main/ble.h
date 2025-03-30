#pragma once

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"
#include "esp_log.h"

#define BLE_ADV_INT_MIN 0x20
#define BLE_ADV_INT_MAX 0x40
#define BLE_SPAM_DEVICE_NAME "macero"

void ble_init(void);
void ble_advertise(void);