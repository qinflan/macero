#pragma once

#include "dns_server.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include <string.h>
#include <unistd.h>

void start_dns_server(void);
void stop_dns_server(void);