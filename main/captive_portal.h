#pragma once

// esp32 device libraries
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

// HTTP server libraries
#include "esp_http_server.h"

// expose main function
void captive_portal_main(void);
void stop_captive_portal(void);