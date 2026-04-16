#include "include/Multiplexer_Class.h"
#include <iostream>
#include <math.h>
#define PORT_NUM I2C_NUM_0

i2c_master_dev_handle_t Multiplexer::dev_handle = {0};
i2c_master_bus_handle_t Multiplexer::bus_handle = {0};
i2c_master_dev_handle_t Multiplexer::accel_handles = {0};

SensorData topLeft;
SensorData topRight;
SensorData middleBack;

#define WYP_DEVICE_NAME "PostureTracker"
constexpr float kPostureThresholdDegrees = 30.0f;
constexpr int kCalibrationSamples = 200;
constexpr int kCalibrationDelayMs = 20;
constexpr int kSampleIntervalMs = 1000;




/*
C-style functions uptop for posture data
*/
constexpr float ACCEL_X_UPRIGHT = -0.70f;
constexpr float ACCEL_Y_UPRIGHT = 0.15f;
constexpr float ACCEL_Z_UPRIGHT = 0.65;
// How to do tests with ESP-IDF on VSCode
// lets you compare values at compile time and test if 
// system / functions are working as intended
float calculatePitch(const AccelData& accel) {
    return (atan2(accel.x, sqrt(accel.y * accel.y  + accel.z * accel.z)) * 180) / M_PI;
    
}
constexpr float calc_pitch(const float &x, const float &y, const float &z){
    return (std::atan2(x, std::sqrt(y * y  + z * z)) * 180) / M_PI;
}
constexpr float calc_roll(const float &y, const float &z){
    return (std::atan2(y, z) * 180) / M_PI;
}

constexpr float baselinePitch = calc_pitch(ACCEL_X_UPRIGHT, ACCEL_Y_UPRIGHT, ACCEL_Z_UPRIGHT);
constexpr float baselineRoll = calc_roll(ACCEL_Y_UPRIGHT, ACCEL_Z_UPRIGHT);



float calculateRoll(const AccelData& accel){
    return (atan2(accel.y, accel.z) * 180) / M_PI;
}

//with calibration tilt in mind, take current tilt deg, set both to abs
//check if current tilt is within acceptable threshold
bool isPostureGood(const float &currentPitch, const float &baselinePitch, const float &currentRoll, const float &baselineRoll){
    const float pitchLowerThreshold = baselinePitch - kPostureThresholdDegrees;
    const float pitchUpperThreshold = baselinePitch + kPostureThresholdDegrees;
    const float rollLowerThreshold = baselineRoll - kPostureThresholdDegrees;
    const float rollUpperThreshold = baselineRoll + kPostureThresholdDegrees;

    bool pitchGood = (currentPitch >= pitchLowerThreshold) && (currentPitch <= pitchUpperThreshold);
    bool rollGood = (currentRoll >= rollLowerThreshold) && (currentRoll <= rollUpperThreshold);
    return pitchGood && rollGood;
}


Multiplexer::Multiplexer():
    accelerometers{
        MPU6050(0x68), 
        MPU6050(0x68),
        MPU6050(0x68),
    }
{

}

void Multiplexer::init(){
    const i2c_master_bus_config_t master_conf = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .intr_priority = 0,
        .flags = {.enable_internal_pullup = 1}
    };

    i2c_new_master_bus(&master_conf,&bus_handle);

    const i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = (uint16_t)MPLEXER_ADDRESS,
        .scl_speed_hz = (uint32_t)100000,
        .scl_wait_us = 0,
        .flags = {.disable_ack_check = 0}

    };
    i2c_master_bus_add_device(bus_handle,&dev_config,&dev_handle);
    
}
void Multiplexer::calculate_accel_combined(float & combined_x, float &combined_y, float &combined_z){
    combined_x = (topLeft.accel.x * 0.4f + topRight.accel.x * 0.4f + middleBack.accel.x *.2f) / (topLeft.accel.x + topRight.accel.x + middleBack.accel.x);
    combined_y = (topLeft.accel.y * 0.4f + topRight.accel.y * 0.4f + middleBack.accel.y *.2f) / (topLeft.accel.y + topRight.accel.y + middleBack.accel.y);
    combined_z = (topLeft.accel.z * 0.4f + topRight.accel.z * 0.4f + middleBack.accel.z *.2f) / (topLeft.accel.z + topRight.accel.z + middleBack.accel.z);
}

void Multiplexer::calculate_gyro_combined(float &combined_x, float &combined_y, float &combined_z){
    combined_x = (topLeft.gyro.x * 0.4f + topRight.gyro.x * 0.4f + middleBack.gyro.x *.2f) / (topLeft.gyro.x + topRight.gyro.x + middleBack.gyro.x);
    combined_y = (topLeft.gyro.y * 0.4f + topRight.gyro.y * 0.4f + middleBack.gyro.y *.2f) / (topLeft.gyro.y + topRight.gyro.y + middleBack.gyro.y);
    combined_z = (topLeft.gyro.z * 0.4f + topRight.gyro.z * 0.4f + middleBack.gyro.z *.2f) / (topLeft.gyro.z + topRight.gyro.z + middleBack.gyro.z);
}

void Multiplexer::start_accels(){
    const i2c_device_config_t accel_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x68,
        .scl_speed_hz = 100000
    };

    i2c_master_bus_add_device(bus_handle, &accel_config, &accel_handles);
    select_channel(0);
    accelerometers[0].init_accel(accel_handles);
    select_channel(1);
    accelerometers[1].init_accel(accel_handles);
    select_channel(2);
    accelerometers[2].init_accel(accel_handles);
}


void Multiplexer::multiplexer_task_thread(void * args){
    Multiplexer * arg = static_cast<Multiplexer*>(args);
    constexpr int kCalibrationSamples = 200;
    constexpr int kCalibrationDelayMs = 20;
    constexpr int kSampleIntervalMs = 1000;
    // Dev handles for both accelerometers
    arg->start_accels();

    float combined_accel_x = 0.0f, combined_accel_y = 0.0f, combined_accel_z = 0.0f;
    float combined_gyro_x = 0.0f, combined_gyro_y = 0.0f, combined_gyro_z = 0.0f;
    while(1){
        for(int i = 0; i < MAX_ACCELEROMETERS; i++){
            printf("Printing Accel Data idx = %d\n", i);
            arg->select_channel(i);
            vTaskDelay(10/portTICK_PERIOD_MS);

            if(i == 0){
                arg->accelerometers[0].read(accel_handles);
                topLeft = arg->accelerometers[0].getSnapshot();
            }else if(i == 1){
                arg->accelerometers[1].read(accel_handles);
                topRight = arg->accelerometers[1].getSnapshot();
            }else if(i == 2){
                arg->accelerometers[2].read(accel_handles);
                middleBack = arg->accelerometers[2].getSnapshot();
            }

        }
        arg->calculate_accel_combined(combined_accel_x, combined_accel_y, combined_accel_z);
        arg->calculate_gyro_combined(combined_gyro_x, combined_gyro_y, combined_gyro_z);
        float currentPitch = calculatePitch({combined_accel_x, combined_accel_y, combined_accel_z});
        float currentRoll = calculateRoll({combined_gyro_x, combined_gyro_y, combined_gyro_z});
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
            combined_accel_x,
            combined_accel_y,
            combined_accel_z,
            combined_gyro_x,
            combined_gyro_y,
            combined_gyro_z,
            topLeft.temp,
            currentPitch,
            baselinePitch,
            pitchError,
            currentRoll,
            baselineRoll,
            rollError,
            postureGood ? "true" : "false");

        vTaskDelay(ONE_TIME_DELAY);

    }
}

void Multiplexer::select_channel(uint8_t channel){
    if(channel > 7){
        return;
    }
    uint8_t cmd = 0b0000'0001 << channel;
    if(i2c_master_transmit(dev_handle,&cmd,sizeof(cmd),ONE_TIME_DELAY) != ESP_OK){
        ESP_LOGE("CHANNEL ERROR:", "Selected channel did not work");
    }
}
