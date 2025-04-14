#include "captive_portal.h"
#include "esp_log.h"

static const char *TAG = "CAPTIVE_PORTAL";

#define CAPTIVE_SSID "ESP32_Captive_Portal"

static void wifi_init_softap(void);
static esp_err_t http_get_handler(httpd_req_t *req);
static httpd_handle_t start_webserver(void);

static httpd_handle_t server = NULL;


// Main function to initilize and start captive portal
void captive_portal_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_softap();
    start_webserver();
}

static void wifi_init_softap(void) {
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CAPTIVE_SSID,
            .ssid_len = strlen(CAPTIVE_SSID),
            .channel = 1,
            .password = "", // open network
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started. SSID: %s", CAPTIVE_SSID);

}

// HTTP get request handler
static esp_err_t http_get_handler(httpd_req_t *req) {
    const char *response = "<html><body><h1>Welcome to the Captive Portal!</h1></body></html>";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// start http server
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_get = {
            .uri = "/*",
            .method = HTTP_GET,
            .handler = http_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get);
    }

    return server;
}

void stop_captive_portal(void) {
    ESP_LOGI(TAG, "Stopping Captive Portal...");

    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    ESP_LOGI(TAG, "Wi-Fi AP stopped");
}
