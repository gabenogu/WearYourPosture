#ifndef MPU_6050_H
#define MPU_6050_H
#include <cstdint>
#include "driver/i2c_master.h"
#include <freertos/FreeRTOS.h>

// Wakeup device to use PLL clock
constexpr uint8_t WAKEUP_REG = 0x6B;
constexpr uint8_t PLL_PWR_BIT_SET = 0x01;

//register + sample rate
constexpr uint8_t SAMPLE_RATE_REG = 0x19;
constexpr uint8_t HZ_125_SAMPLERATE = 0x07;

// Low pass filter register + value
constexpr uint8_t DLPF_REGISTER = 0x1A;
constexpr uint8_t DLPF_VALUE = 0x03;

// Gyro config for +- 250 deg/s(sensitivity 131.0 LSB/s)
constexpr uint8_t GYRO_RATE_REG = 0x1B;
constexpr uint8_t GYRO_RATE_VALUE = 0x00;

// Accel config for +- 2g (sensitivity 16384.0 LSB/s)
constexpr uint8_t ACCEL_RATE_REG = 0x1C;
constexpr uint8_t ACCEL_RATE_VALUE = 0x00;

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
        

        //complementary filter function
        void compute_acc_angles(SensorData *data, float &acc_pitch, float &acc_roll);
        void update_orientation(SensorData *data, float dt);
        void print_data();

        // Estimated angles (can also be class members if needed)
        float pitch = 0.0f;
        float roll  = 0.0f;
        void read(i2c_master_dev_handle_t i2c_dev); 
};
#endif