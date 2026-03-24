#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"
#include "esp_log.h"
#include "ble_config.h"
#include "ble_data.h"
#include "ble_gatt.h"

uint8_t posture_score = 100; // Global variable to store the posture score
uint8_t posture_status = 1; // Global variable to store the posture status, 1 is good, 0 is bad
// Define the UUIDs for the posture service and its characteristics
static const ble_uuid128_t posture_service_uuid =
    BLE_UUID128_INIT(0x34, 0xf7, 0xc7, 0xfb,
                     0x7c, 0x81, 0x36, 0xa1,
                     0x09, 0x4b, 0x2b, 0x43,
                     0x29, 0xeb, 0x96, 0xce);
static const ble_uuid128_t posture_score_uuid =
    BLE_UUID128_INIT(0x34, 0x52, 0x31, 0xfb,
                     0xe0, 0x81, 0xf1, 0xbb,
                     0x28, 0x48, 0x41, 0xac,
                     0x16, 0x73, 0x27, 0xf9);
static const ble_uuid128_t posture_status_uuid =
    BLE_UUID128_INIT(0xc5, 0xaf, 0x94, 0x94,
                     0xc0, 0x4a, 0x1c, 0x9e,
                     0x20, 0x4c, 0x72, 0xff,
                     0x9a, 0x2e, 0x1c, 0x3f);

//Read access callbacks
static int posture_score_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    int rc = os_mbuf_append(ctxt->om, &posture_score, sizeof(posture_score)); // Append the posture score to the GATT response
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}
static int posture_status_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    int rc = os_mbuf_append(ctxt->om, &posture_status, sizeof(posture_status)); // Append the posture status to the GATT response
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}
//Characteristics table
static const struct ble_gatt_chr_def posture_characteristics[] = {
    {
        .uuid = &posture_score_uuid.u,
        .access_cb = posture_score_access_cb, // Set the access callback for the posture score characteristic
        .flags = BLE_GATT_CHR_F_READ, // Set the characteristic to be readable
    },
    {
        .uuid = &posture_status_uuid.u,
        .access_cb = posture_status_access_cb, // Set the access callback for the posture status characteristic
        .flags = BLE_GATT_CHR_F_READ, // Set the characteristic to be readable
    },
    {
        0, 
    }
};
//Service table
static const struct ble_gatt_svc_def posture_service[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY, //This sets the service type to primary
        .uuid = &posture_service_uuid.u, //Sets the service UUID to the defined posture service UUID
        .characteristics = posture_characteristics,
    },
    {
        0,
    }
};
void ble_gatt_init() {
    // Initialize GATT services and characteristics here
    // This function can be expanded to set up the GATT server and define characteristics for posture score and status

}

