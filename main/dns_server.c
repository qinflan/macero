#include "dns_server.h"
#include "esp_log.h"
#include <unistd.h>

#define DNS_PORT 53
#define DNS_REPLY_SIZE 512 //size of packets

static const char *TAG = "DNS_SERVER";
static const uint8_t RESPONSE_IP[4] = {192, 168, 4, 1};

static int sock = -1; // global socket variable to track dns server socket
static bool dns_server_running = false; // flag to track DNS server state



void start_dns_server(void) {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DNS_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed");
        close(sock);
        return;
    }

    dns_server_running = true;
    ESP_LOGI(TAG, "DNS server started on port %d", DNS_PORT);

    uint8_t buffer[DNS_REPLY_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (dns_server_running) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);

        if (len < 0) {
            ESP_LOGE(TAG, "revcfrom failed");
            continue;
        }

        // prepare dns response by copying header
        uint8_t response[DNS_REPLY_SIZE];
        memcpy(response, buffer, len);

        response[2] = 0x81;
        response[3] = 0x80;

        response[6] = 0x00;
        response[7] = 0x01;

        int i = 12;
        while (i < len && buffer[i] != 0) i++;
        i += 5;

        int ans_start = i;

        // query pointer
        response[i++] = 0xC0;
        response[i++] = 0x0C;

        // ipv4
        response[i++] = 0x00;
        response[i++] = 0x01;

        // internet
        response[i++] = 0x00;
        response[i++] = 0x01;

        // TTL (4 bytes)
        response[i++] = 0x00;
        response[i++] = 0x00;
        response[i++] = 0x00;
        response[i++] = 0x3C; 

        // data length
        response[i++] = 0x00;
        response[i++] = 0x04;

        // ESP32 IP address
        memcpy(&response[i], RESPONSE_IP, 4);
        i += 4;

        // send response
        sendto(sock, response, i, 0, (struct sockaddr *)&client_addr, addr_len);

    }
}

void stop_dns_server(void) {
    if (sock >= 0) {
        ESP_LOGI(TAG, "Stopping DNS server...");
        dns_server_running = false;
        close(sock); 
        sock = -1;
        ESP_LOGI(TAG, "DNS server stopped");
    } else {
        ESP_LOGW(TAG, "DNS server not running");
    }
}