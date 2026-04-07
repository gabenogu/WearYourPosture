//BLE GAP Header file

#ifndef BLUETOOTH_WYP_H
#define BLUETOOTH_WYP_H

#include <stdint.h> // fixed size interger types
#include "host/ble_hs.h" // BLE host stack header

#define WYP_DEVICE_NAME "POstureTracker"

extern uint16_t ble_conn_handle; // Global variable to store the connection handle

void ble_gap_init(void); // Function to initialize BLE GAP
void ble_gap_start_advertising(void); // Function to start BLE advertising

#endif // BLUETOOTH_WYP_H
