#include "include/ble_gatt.h"
#include "include/ble_data.h"

uint8_t posture_score = 100; // Global variable to store the posture score
uint8_t posture_status = 1; // Global variable to store the posture status
struct ble_gatt_chr_def{
    uint8_t *uuid;
    uint8_t *value;
};
uint8_t read_posture_score() {
    return posture_score; // Function to read the current posture score
}
uint8_t read_posture_status() {
    return posture_status; // Function to read the current posture status
}
void ble_gatt_init() {
    // Initialize GATT services and characteristics here
    // This function can be expanded to set up the GATT server and define characteristics for posture score and status
    static const ble_gatt_chr_def posture_score_char = {
        .uuid = (uint8_t *)"posture_score_uuid", // Placeholder UUID for posture score characteristic
        .value = (uint8_t *)&posture_score // Point to the posture score variable
    };

}

