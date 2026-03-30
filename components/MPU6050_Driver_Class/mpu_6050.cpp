#include "mpu_6050.h"

#define ACCEL_SCALE 16384.0f 
#define GYRO_SCALE 131.0f
#define CONVERT_TEMP 340.0f + 36.53f
MPU6050::MPU6050(uint8_t address): slave_addr(address){
    // Initialize the sensor (e.g., wake it up)
    

}
void MPU6050::init_accel( i2c_master_dev_handle_t i2c_dev){
    // 1. Wake up + use PLL (VERY IMPORTANT)
    constexpr uint8_t pwr[2] = {WAKEUP_REG, PLL_PWR_BIT_SET};
    i2c_master_transmit(i2c_dev, pwr, 2, 100);

    vTaskDelay(10 / portTICK_PERIOD_MS);

    // 2. Set sample rate divider (1kHz / (1 + 7) = 125 Hz)
    constexpr uint8_t smplrt[2] = {SAMPLE_RATE_REG, HZ_125_SAMPLERATE};
    i2c_master_transmit(i2c_dev, smplrt, 2, 100);

    // 3. Configure DLPF (important for stable data updates)
    constexpr uint8_t config[2] = {DLPF_REGISTER, DLPF_VALUE};
    i2c_master_transmit(i2c_dev, config, 2, 100);

    // 4. Gyro config (±250 deg/s)
    constexpr uint8_t gyro[2] = {GYRO_RATE_REG,GYRO_RATE_VALUE};
    i2c_master_transmit(i2c_dev, gyro, 2, 100);

    // 5. Accel config (±2g)
    constexpr uint8_t accel[2] = {ACCEL_RATE_REG, ACCEL_RATE_VALUE};
    i2c_master_transmit(i2c_dev, accel, 2, 100);
}

static inline int16_t combine(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}


void MPU6050::read(i2c_master_dev_handle_t i2c_dev) {
    // Read 14 bytes of data starting from the DATA_START_REG
    uint8_t buffer[14];
    uint8_t reg = DATA_START_REG;
    i2c_master_transmit_receive(i2c_dev,&reg,sizeof(reg),buffer,14,100);
    
    // Convert the raw data to signed integers
    data.accel.x = combine(buffer[0], buffer[1]) /(ACCEL_SCALE);
    data.accel.y = combine(buffer[2], buffer[3])/(ACCEL_SCALE);
    data.accel.z = combine(buffer[4], buffer[5]) /(ACCEL_SCALE);

    data.temp = combine(buffer[6], buffer[7]) / (CONVERT_TEMP);

    data.gyro.x = combine(buffer[8], buffer[9]) / (GYRO_SCALE);
    data.gyro.y = combine(buffer[10], buffer[11]) / (GYRO_SCALE);
    data.gyro.z = combine(buffer[12], buffer[13]) / (GYRO_SCALE);
    printf("Accel: X=%f, Y=%f, Z=%f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%f, Y=%f, Z=%f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Temp: %f\n", data.temp);
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
    pitch = ALPHA * (pitch + data->gyro.x * dt) + (1.0f - ALPHA) * acc_pitch;
    roll  = ALPHA * (roll  + data->gyro.y * dt) + (1.0f - ALPHA) * acc_roll;
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
