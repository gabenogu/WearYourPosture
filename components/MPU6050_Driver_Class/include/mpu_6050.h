#ifndef MPU_6050_H
#define MPU_6050_H
#include <cstdint>
#include "driver/i2c_master.h"
#include <freertos/FreeRTOS.h>

#define WAKEUP_REG    0x6B
#define DATA_START_REG 0x3B
#define ONE_TIME_DELAY (1000 / portTICK_PERIOD_MS)

struct AccelData {

    float x;
    float y;
    float z;

};
struct GyroData {  

    float x;
    float y;
    float z;
};

struct SensorData {

    AccelData accel;
    GyroData gyro;
    float temp;
    /*SensorState state;*/

};


// void sensor_init(void);
// void sensor_read(AccelData *data);


class MPU6050 {
    /*this is a function: construct  will insitailzie object and ic2 bus 
    have a read fucntion and its what does the reading from the device*/

    private: 
        uint8_t slave_addr;
        SensorData data;


    public:
        MPU6050() = default;
        explicit MPU6050(uint8_t address);
        void init_accel(i2c_master_dev_handle_t i2c_dev);
        void read(i2c_master_dev_handle_t i2c_dev); 
};

#endif
