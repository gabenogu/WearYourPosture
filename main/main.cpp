#include <stdio.h>
#include "mpu_6050.h"
#include "Bluetooth_WYP.h"
#include <mutex>
#include <iostream>

extern "C" void app_main(void){

    //Initialize Bluetooth on ESP32
    ble_gap_init();
    
    // Initialize the MPU6050 sensor
    MPU6050 sensor(I2C_NUM_0, 0x68); // Use I2C port 0 and address 0x68

    while (true) {
        print_data(sensor);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}