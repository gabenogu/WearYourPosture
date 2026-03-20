#include "mpu_6050.h"
#include "Bluetooth_WYP.h"
#include <iostream>
#include <freertos/task.h>

extern "C" void app_main(void){

    //Initialize Bluetooth on ESP32
    //ble_gap_init();

    // Initialize the MPU6050 sensor
    static MPU6050 sensor(I2C_NUM_0, 0x68); // Use I2C port 0 and address 0x68
    
    
    xTaskCreate(&MPU6050::delegate_read, "Accel 1 Read", 4096, &sensor, configMAX_PRIORITIES-1,NULL);
}