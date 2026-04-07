#ifndef WIFI_WYP_H
#define WIFI_WYP_H

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_FAILED
} wifi_state_t;

extern wifi_state_t wifi_connection_state; // Variable to store state of wifi connection

void wifi_driver_init(const char* ssid, const char* password); // Initializes wifi driver

bool wifi_is_connected(void); // Checks if wifi is connected

#endif