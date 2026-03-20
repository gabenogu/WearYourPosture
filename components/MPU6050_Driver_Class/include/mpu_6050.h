#ifndef MPU_6050_H
#define MPU_6050_H
#include <stdint.h>
#include "driver/i2c.h"

#define WAKEUP_REG    0x6B
#define DATA_START_REG 0x3B

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
        i2c_port_t i2c_port;
        uint8_t slave_addr;


    public:
        MPU6050(i2c_port_t port, uint8_t address);
        esp_err_t init();
        void read(SensorData *data); 

        friend void print_data(MPU6050 &sensor);

    
};

void print_data(MPU6050 &sensor);
#endif
