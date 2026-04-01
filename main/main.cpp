#include "mpu_6050.h"
#include "Bluetooth_WYP.h"
#include <iostream>
#include "postureLogic.h"

extern "C" void app_main(void){

    //Initialize Bluetooth on ESP32
    ble_gap_init();

    // Initialize the MPU6050 sensor
    MPU6050 sensor(I2C_NUM_0, 0x68); // Use I2C port 0 and address 0x68

    SensorData data{};
    float baselinePitch = 0.0f;
    float baselineRoll = 0.0f;
    const int calibrationSamples = 200;

    for (int i = 0; i < calibrationSamples; ++i) {
        sensor.read(&data);
        baselinePitch += calculatePitch(data.accel);
        baselineRoll += calculateRoll(data.accel);
        
        vTaskDelay(20 / portTICK_PERIOD_MS); // Delay for 20 ms between samples

       
    }

    baselinePitch /= calibrationSamples;
    baselineRoll /= calibrationSamples;

    while (true) {
        sensor.read(&data);
        float currentPitch = calculatePitch(data.accel);
        float currentRoll = calculateRoll(data.accel);

        float pitchError = currentPitch - baselinePitch;
        float rollError = currentRoll - baselineRoll;

        
        bool postureGood = isPostureGood(currentPitch, baselinePitch,currentRoll, baselineRoll);

        printf("Current Pitch: %f, Baseline Pitch: %f, Pitch Error: %f, Current Roll: %f, Baseline Roll: %f, Roll Error: %f, Posture Good: %s\n", 
            currentPitch, 
            baselinePitch, 
            pitchError,
            currentRoll,
            baselineRoll,
            rollError,
            postureGood ? "Yes" : "No");
        
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }



    // while (true) {
    //     print_data(sensor);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    // }
}