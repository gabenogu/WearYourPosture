#include "mpu_6050.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ACCEL_SCALE 16384.0f 
#define GYRO_SCALE 131.0f
#define CONVERT_TEMP 340.0f + 36.53f
#define ALPHA 0.98f

MPU6050::MPU6050(uint8_t address): slave_addr(address){
    // Initialize the sensor (e.g., wake it up)
    

}
void MPU6050::init_accel( i2c_master_dev_handle_t i2c_dev){
    // 1. Wake up + use PLL (VERY IMPORTANT)
    constexpr uint8_t pwr[2] = {WAKEUP_REG, PLL_PWR_BIT_SET};
    i2c_master_transmit(i2c_dev, pwr, 2, 100);

    vTaskDelay(10 / portTICK_PERIOD_MS);


    uint8_t wake_cmd[2] = {WAKEUP_REG, 0x00};
    i2c_master_write_to_device(i2c_port, slave_addr, wake_cmd, 2, pdMS_TO_TICKS(100));

    uint8_t config_register[2] = {0x1C, 0x00};
    i2c_master_write_to_device(i2c_port, slave_addr, config_register, 2, pdMS_TO_TICKS(100));

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
<<<<<<< HEAD
    i2c_master_write_read_device(i2c_port,slave_addr, &reg, 1, buffer, 14, pdMS_TO_TICKS(100));
=======
    i2c_master_transmit_receive(i2c_dev,&reg,sizeof(reg),buffer,14,100);
>>>>>>> b7938ccebc6544422e5e5956b76e48242c419b95
    
    // Convert the raw data to signed integers
    data.accel.x = combine(buffer[0], buffer[1]) /(ACCEL_SCALE);
    data.accel.y = combine(buffer[2], buffer[3])/(ACCEL_SCALE);
    data.accel.z = combine(buffer[4], buffer[5]) /(ACCEL_SCALE);

    data.temp = combine(buffer[6], buffer[7]) / (CONVERT_TEMP);

    data.gyro.x = combine(buffer[8], buffer[9]) / (GYRO_SCALE);
    data.gyro.y = combine(buffer[10], buffer[11]) / (GYRO_SCALE);
    data.gyro.z = combine(buffer[12], buffer[13]) / (GYRO_SCALE);
    printf("Accel: X=%.2f, Y=%.2f, Z=%.2f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Pitch: %.2f, Roll: %.2f\n", pitch, roll);
    printf("Temp: %.2f\n", data.temp);
}

void MPU6050::calibrate_gyro(int samples){
    SensorData data;
    float sum_x = 0, sum_y = 0;

    for(int i = 0; i < samples; i++){
        read(&data);

        sum_x += data.gyro.x;
        sum_y += data.gyro.y;

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    gyro_bias_x = sum_x / samples;
    gyro_bias_y = sum_y / samples;
}


//compute pitch and roll from accelerometer
void MPU6050:: compute_acc_angles(SensorData *data, float &acc_pitch, float &acc_roll){
    acc_pitch = atan2(data->accel.y, sqrt(data->accel.x*data->accel.x + data->accel.z*data->accel.z)) * 180.0 / M_PI;
    acc_roll  = atan2(-data->accel.x, data->accel.z) * 180.0 / M_PI;
}
//complementar filter & error calculation 
void MPU6050::update_orientation(SensorData *data, float dt){

    float gyro_x = data->gyro.x - gyro_bias_x;
    float gyro_y = data->gyro.y - gyro_bias_y;

    float acc_pitch, acc_roll;
    compute_acc_angles(data, acc_pitch, acc_roll);

<<<<<<< HEAD
    // complementary filter
    pitch = ALPHA * (pitch + gyro_x * dt) + (1 - ALPHA) * acc_pitch;
    roll  = ALPHA * (roll  + gyro_y * dt) + (1 - ALPHA) * acc_roll;

    // error propagation (simple version)
    float gyro_var = 0.05f * 0.05f;
    float acc_var  = 0.5f * 0.5f;

    pitch_var = ALPHA * ALPHA * (pitch_var + gyro_var * dt * dt) + (1 - ALPHA) * (1 - ALPHA) * acc_var;

    roll_var  = ALPHA * ALPHA * (roll_var + gyro_var * dt * dt) + (1 - ALPHA) * (1 - ALPHA) * acc_var;
}
void print_data(MPU6050 &sensor) {
    SensorData data;
    sensor.read(&data);
    printf("Accel: X=%.2f, Y=%.2f, Z=%.2f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Pitch: %.2f, Roll: %.2f\n", sensor.pitch, sensor.roll);
    printf("Temp: %.2f\n", data.temp);
    //use %.2f bc prints a float w 2 decimal places instead of %f --> 6 decimal places

=======
    // Integrate gyro (dead reckoning)
    pitch = ALPHA * (pitch + data->gyro.x * dt) + (1.0f - ALPHA) * acc_pitch;
    roll  = ALPHA * (roll  + data->gyro.y * dt) + (1.0f - ALPHA) * acc_roll;
>>>>>>> b7938ccebc6544422e5e5956b76e48242c419b95
}
