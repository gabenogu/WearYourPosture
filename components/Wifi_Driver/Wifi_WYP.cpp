#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>
#include "Wifi_WYP.h"

static const char *TAG = "Wifi_WYP";

#define MAX_RETRY_COUNT 5 
static int retry_count = 0;

wifi_state_t wifi_connection_state = WIFI_STATE_DISCONNECTED; // current state of wifi

// event handler function
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if (event_base == WIFI_EVENT) {

        switch (event_id) {
            case WIFI_EVENT_STA_START: // Station started, attempt to connect
                ESP_LOGI(TAG, "WiFi started, connecting...");
                wifi_connection_state = WIFI_STATE_CONNECTING;
                esp_wifi_connect(); // Attempt to connect to the configured WiFi network
                break;
            case WIFI_EVENT_STA_CONNECTED:// Successfully connected to WiFi
                ESP_LOGI(TAG, "WiFi connected");
                retry_count = 0;
                break;
            case WIFI_EVENT_STA_DISCONNECTED: // Station disconnected
                wifi_connection_state = WIFI_STATE_DISCONNECTED;
                if (retry_count < MAX_RETRY_COUNT){ // If we haven't exceeded max retries, try to reconnect
                    retry_count++;
                    ESP_LOGE(TAG, "WiFi disconnected, retrying... (%d/%d)", retry_count, MAX_RETRY_COUNT);
                    esp_wifi_connect();
                } else {
                    wifi_connection_state = WIFI_STATE_FAILED;
                    ESP_LOGE(TAG, "WiFi connection failed after %d attempts", MAX_RETRY_COUNT);
                }
                break;
        }
    } else if (event_base == IP_EVENT){ // IP event, check if we got an IP address
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data; // Cast event data to the correct type
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip)); // Log the obtained IP address
                wifi_connection_state = WIFI_STATE_CONNECTED;
                retry_count = 0;
                break;
        }
    }
}

bool wifi_is_connected(void) {
    return wifi_connection_state == WIFI_STATE_CONNECTED;
}

// Initializes the WiFi driver and starts the connection process
void wifi_driver_init(const char *ssid, const char *password) {
    esp_err_t ret = nvs_flash_init(); // Initialize NVS flash
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){ // If there is an NVS issue, erase and reinitialize
        ESP_LOGW(TAG, "NVS issue, erasing and retrying...");
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_netif_init(); // Initialize the TCP/IP stack
    esp_event_loop_create_default(); // Create default event loop
    esp_netif_create_default_wifi_sta(); // Create default WiFi station network interface

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Get default WiFi configuration
    esp_wifi_init(&cfg); // Initialize WiFi with the default configuration

    // Register event handlers for WiFi and IP events
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config; // Create a wifi_config_t structure to hold the WiFi configuration

    // Set the WiFi configuration with the provided SSID and password
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi initialization complete.");
    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
    ESP_LOGI(TAG, "Connection result will appear in event handler...");
}