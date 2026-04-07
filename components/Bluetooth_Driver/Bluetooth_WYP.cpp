// BLE GAP logic implementation file
#include "nvs_flash.h" // Non-volatile storage
#include "nimble/nimble_port.h" // NimBLE port header
#include "nimble/nimble_port_freertos.h" // NimBLE FreeRTOS
#include "host/ble_hs.h" // BLE host stack header
#include "services/gap/ble_svc_gap.h" // GAP service header
#include "esp_log.h" // ESP logging header
#include "Bluetooth_WYP.h" // BLE GAP header

static const char *TAG = "BLE_GAP"; // Tag for logging
static uint8_t own_addr_type; // Variable to store the device's own address type
uint16_t ble_conn_handle = BLE_HS_CONN_HANDLE_NONE; // conn_handle is the ID of the currently connected phone. BLE_HS_CONN_HANDLE_NONE = 0xFFFF, which means no phone is connected. 

static int gap_event_handler(struct ble_gap_event *event, void *arg);

// Function: Freertos task for running the BLE host stack
static void ble_host_task(void *param) {
    ESP_LOGI(TAG, "BLE host task started");
    nimble_port_run(); // Run the NimBLE host task
    nimble_port_freertos_deinit(); // Deinitialize NimBLE FreeRTOS
}

// Function: Initialize and configures BLE advertising
void ble_gap_start_advertising(void){
    struct ble_hs_adv_fields fields = {0}; // Structure to hold advertising fields the phone receives when it scans for nearby devices (the content)
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP; // Set advertising flags: general discoverable mode and blue classic not supported

    // Set the device name in the advertising data
    const char *name = ble_svc_gap_device_name(); // Get the device name from the GAP service
    fields.name = (const uint8_t *)name; // Set the name field in the advertising data
    fields.name_len = strlen(name); // Set the length of the name field
    fields.name_is_complete = 1; // Indicate that the name field is complete

    // This function converts data in to raw bytes
    int rc = ble_gap_adv_set_fields(&fields); // Set the advertising fields
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting advertisement data: %d", rc); // Log an error if setting the advertising fields fails
        return;
    }

    struct ble_gap_adv_params adv_params = {0}; // Structure to hold advertising parameters (the behavior)
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // Set connection mode to undirected connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // Set discoverable mode
    
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event_handler, NULL); // Start advertising with the specified parameters
    if (rc != 0) {
        ESP_LOGE(TAG, "Error starting advertising: %d", rc); // Log an error if starting advertising fails
        return;
    }

    ESP_LOGI(TAG, "Advertising started successfully with device: %s", name); // Log that advertising has started successfully

}

static int gap_event_handler(struct ble_gap_event *event, void *arg){

    // checks the type of event
    switch (event->type){
        case BLE_GAP_EVENT_CONNECT: // Handles the event when connection is established
            if (event->connect.status == 0) {
                ble_conn_handle = event->connect.conn_handle; // Store the connection handle of the connected phone
                ESP_LOGI(TAG, "Device connected, conn_handle: %d", ble_conn_handle); // Log that a device has connected
            } else {
                ble_conn_handle = BLE_HS_CONN_HANDLE_NONE; // Reset the connection handle if the connection attempt failed
                ESP_LOGE(TAG, "Connection failed with status: %d", event->connect.status); // Log the error status of the failed connection attempt
                ble_gap_start_advertising(); // Restart advertising to allow new connection attempts
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT: // Handles the event when connection is terminated
            ESP_LOGI(TAG, "Device disconnected, reason: %d", event->disconnect.reason); // Log that a device has disconnected and the reason for disconnection
            ble_conn_handle = BLE_HS_CONN_HANDLE_NONE; // Reset the connection handle
            ble_gap_start_advertising(); // Restart advertising to allow new connection attempts
            break; 

        case BLE_GAP_EVENT_ADV_COMPLETE: // Handles the event when advertsing is complted
            ESP_LOGI(TAG, "Advertising completed, reason: %d", event->adv_complete.reason); // Log that advertising has completed and the reason for completion
            ble_gap_start_advertising(); // Restart advertising to allow new connection attempts
            break;
    }
    return 0;
}

// This function is called when the BLE stack is synchronized and ready to use
static void on_sync(void){
    ESP_LOGI(TAG, "BLE stack synchronized"); // Log that the BLE stack is synchronized
    int rc = ble_hs_id_infer_auto(0, &own_addr_type); // Automatically determine the device's own address type
    if (rc != 0) {
        ESP_LOGE(TAG, "Error determining own address type: %d", rc); // Log an error if determining the own address type fails
        return;
    }

    ble_gap_start_advertising(); // Start advertising to allow connection attempts
}

static void on_reset(int reason){
    ESP_LOGI(TAG, "BLE stack reset, reason: %d", reason); // Log that the BLE stack has been reset and the reason for the reset
}


void ble_gap_init(void){
    esp_err_t ret = nvs_flash_init(); // Initialize non-volatile storage
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS flash erasing and reinitializing"); // Log a warning if NVS flash needs to be erased and reinitialized
        nvs_flash_erase(); // Erase NVS flash
        nvs_flash_init(); // Reinitialize NVS flash
    }

    nimble_port_init(); // Initialize the NimBLE port

    ble_hs_cfg.sync_cb = on_sync; // Set the synchronization callback to be called when the BLE stack is ready
    ble_hs_cfg.reset_cb = on_reset; // Set the reset callback to be called when the BLE stack is reset

    ble_svc_gap_init(); // Initialize the GAP service
    ble_svc_gap_device_name_set(WYP_DEVICE_NAME); // Set the device name for the GAP service

    nimble_port_freertos_init(ble_host_task); // Initialize the NimBLE FreeRTOS task to run the BLE host stack

    ESP_LOGI(TAG, "BLE GAP initialized successfully"); // Log that BLE GAP has been initialized successfully

}
