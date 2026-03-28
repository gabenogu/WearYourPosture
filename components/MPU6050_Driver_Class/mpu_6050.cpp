#include "mpu_6050.h"
#include <math.h>

#define ACCEL_SCALE 16384.0f 
#define GYRO_SCALE 131.0f
#define ONE_TIME_DELAY (1000 / portTICK_PERIOD_MS)
#define CONVERT_TEMP 340.0f + 36.53f

#define ALPHA 0.98f
//bc trust gyro (98%) for fast, smooth respone and rely 
// on accelerometer (2%) to correct long term drift



MPU6050::MPU6050(i2c_port_t port, uint8_t address){
    // Initialize the sensor (e.g., wake it up)
    i2c_port = port;
    slave_addr = address;

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_21; 
    conf.scl_io_num = GPIO_NUM_22; 
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;

    i2c_param_config(i2c_port, &conf);
    i2c_driver_install(i2c_port, conf.mode, 0, 0,0);

    uint8_t wake_cmd[2] = {WAKEUP_REG, 0x00};
    i2c_master_write_to_device(i2c_port, slave_addr, wake_cmd, sizeof(wake_cmd), ONE_TIME_DELAY); 
    const uint8_t config_register[2] = {0x1C,0x00};
    i2c_master_write_to_device(i2c_port, slave_addr, config_register, sizeof(config_register), ONE_TIME_DELAY);
}

inline int16_t combine(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}


void MPU6050:: read(SensorData *data) {
    // Read 14 bytes of data starting from the DATA_START_REG
    uint8_t buffer[14];
    uint8_t reg = DATA_START_REG;
    i2c_master_write_read_device(i2c_port,slave_addr, &reg, 1, buffer, 14, ONE_TIME_DELAY);
    
    // Convert the raw data to signed integers
    data->accel.x = combine(buffer[0], buffer[1]) /(ACCEL_SCALE);
    data->accel.y = combine(buffer[2], buffer[3])/(ACCEL_SCALE);
    data->accel.z = combine(buffer[4], buffer[5]) /(ACCEL_SCALE);

    data->temp = combine(buffer[6], buffer[7]) / (CONVERT_TEMP);

    data->gyro.x = combine(buffer[8], buffer[9]) / (GYRO_SCALE);
    data->gyro.y = combine(buffer[10], buffer[11]) / (GYRO_SCALE);
    data->gyro.z = combine(buffer[12], buffer[13]) / (GYRO_SCALE);
}

//compute pitch and roll from accelerometer
void MPU6050:: compute_acc_angles(SensorData *data, float &acc_pitch, float &acc_roll){
    acc_pitch = atan2(data->accel.y, sqrt(data->accel.x*data->accel.x + data->accel.z*data->accel.z)) * 180.0 / M_PI;
    acc_roll  = atan2(-data->accel.x, data->accel.z) * 180.0 / M_PI;
}
//complementar filter
void MPU6050::update_orientation(SensorData *data, float dt){
    float acc_pitch, acc_roll;
    compute_acc_angles(data, acc_pitch, acc_roll);

    // Integrate gyro (dead reckoning)
    this->pitch = ALPHA * (this->pitch + data->gyro.x * dt) + (1.0f - ALPHA) * acc_pitch;
    this->roll  = ALPHA * (this->roll  + data->gyro.y * dt) + (1.0f - ALPHA) * acc_roll;
}

void print_data(MPU6050 &sensor) {
    SensorData data;
    sensor.read(&data);
    printf("Accel: X=%.2f, Y=%.2f, Z=%.2f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Pitch: %.2f, Roll: %.2f\n", sensor.pitch, sensor.roll);
    printf("Temp: %.2f\n", data.temp);
    //use %.2f bc prints a float w 2 decimal places instead of %f --> 6 decimal places

}
