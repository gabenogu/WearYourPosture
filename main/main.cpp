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
    constexpr int kCalibrationSamples = 200;
    constexpr int kCalibrationDelayMs = 20;
    constexpr int kSampleIntervalMs = 1000;

    for (int i = 0; i < kCalibrationSamples; ++i) {
        sensor.read(&data);
        baselinePitch += calculatePitch(data.accel);
        baselineRoll += calculateRoll(data.accel);
        
        vTaskDelay(kCalibrationDelayMs / portTICK_PERIOD_MS); // Delay for 20 ms between samples

       
    }

    baselinePitch /= kCalibrationSamples;
    baselineRoll /= kCalibrationSamples;

    while (true) {
        sensor.read(&data);
        float currentPitch = calculatePitch(data.accel);
        float currentRoll = calculateRoll(data.accel);

        float pitchError = currentPitch - baselinePitch;
        float rollError = currentRoll - baselineRoll;

        
        bool postureGood = isPostureGood(currentPitch, baselinePitch,currentRoll, baselineRoll);

        printf(
            "WYP_TELEMETRY {\"deviceName\":\"%s\",\"sampleIntervalMs\":%d,\"calibrationSamples\":%d,"
            "\"thresholdDegrees\":%.1f,"
            "\"accel\":{\"x\":%.4f,\"y\":%.4f,\"z\":%.4f},"
            "\"gyro\":{\"x\":%.4f,\"y\":%.4f,\"z\":%.4f},"
            "\"temp\":%.4f,"
            "\"currentPitch\":%.4f,\"baselinePitch\":%.4f,\"pitchError\":%.4f,"
            "\"currentRoll\":%.4f,\"baselineRoll\":%.4f,\"rollError\":%.4f,"
            "\"postureGood\":%s}\n",
            WYP_DEVICE_NAME,
            kSampleIntervalMs,
            kCalibrationSamples,
            kPostureThresholdDegrees,
            data.accel.x,
            data.accel.y,
            data.accel.z,
            data.gyro.x,
            data.gyro.y,
            data.gyro.z,
            data.temp,
            currentPitch,
            baselinePitch,
            pitchError,
            currentRoll,
            baselineRoll,
            rollError,
            postureGood ? "true" : "false");
        
        vTaskDelay(kSampleIntervalMs / portTICK_PERIOD_MS); // Delay for 1 second
    }



    // while (true) {
    //     print_data(sensor);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    // }
}
