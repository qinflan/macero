#pragma once

// esp32 device libraries
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"

// freeRTOS for multi-tasking
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

void start_beacon_flood(void); 
