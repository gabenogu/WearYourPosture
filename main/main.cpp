#include "mpu_6050.h"
#include "Bluetooth_WYP.h"
#include <iostream>
#include "Multiplexer_Class.h"
#include <freertos/task.h>

extern "C" void app_main(void){

    //Initialize Bluetooth on ESP32
    //ble_gap_init();


    // static Multiplexer mult;
    // mult.init();
    const uint8_t addr = 0x68;
    static MPU6050 accel(addr);

    const 

    accel.
    // Initialize the MPU6050 sensor
    // xTaskCreate(mult.multiplexer_task_thread,"Multiplexer Task", 4096, &mult,configMAX_PRIORITIES-1,NULL);
}