#include <stdio.h>
#include "mpu_6050.h"
#include <mutex>
#include <iostream>
#include "freertos/task.h"

extern "C" void app_main(void){
    // Initialize the MPU6050 sensor
    MPU6050 sensor(I2C_NUM_0, 0x68, 0x70, 0); // Use I2C port 0 and address 0x68

    if (sensor.init() != ESP_OK) {
        printf("Failed to initialize MPU6050\n");
        return;
    }

    while (true) {
        print_data(sensor);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}
