#include "mpu_6050.h"
#include <math.h>
#include "esp_log.h"

#define ACCEL_SCALE 16384.0f 
#define GYRO_SCALE 131.0f
#define CONVERT_TEMP 340.0f + 36.53f
#define ALPHA 0.98f

static const char *TAG = "MPU6050";

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

    remap_axes(&data); // Remap axes if needed

    printf("Accel: X=%.2f, Y=%.2f, Z=%.2f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Pitch: %.2f, Roll: %.2f\n", pitch, roll);
    printf("Temp: %.2f\n", data.temp);
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

// Gyro calibration function
void MPU6050::calibrate_gyro(i2c_master_dev_handle_t i2c_dev){
    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;

    printf("Starting gyro calibration... Keep the device still.\n");

    for (int i = 0; i < CALIBRATION_SAMPLES; i++){
        read(i2c_dev);
        sum_x += data.gyro.x;
        sum_y += data.gyro.y;
        sum_z += data.gyro.z;
        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay between samples
    }
    gyro_bias_x = sum_x / CALIBRATION_SAMPLES;
    gyro_bias_y = sum_y / CALIBRATION_SAMPLES;
    gyro_bias_z = sum_z / CALIBRATION_SAMPLES;

    printf("Gyro Calibration Complete: Bias X=%.2f, Y=%.2f, Z=%.2f\n", gyro_bias_x, gyro_bias_y, gyro_bias_z);
}

void MPU6050::remap_axes(SensorData *data){
    float raw_x = data->accel.x;
    float raw_y = data->accel.y;
    float raw_z = data->accel.z;

    float raw_gx = data->gyro.x;
    float raw_gy = data->gyro.y;
    float raw_gz = data->gyro.z;

    data->accel.x = raw_x; // New X is old Y
    data->accel.y = raw_z; // New Y is old X
    data->accel.z = raw_y; // Z remains the same

    data->gyro.x = raw_gx; // New X is old Y
    data->gyro.y = raw_gz; // New Y is old X
    data->gyro.z = raw_gy; // Z remains the same
}


void MPU6050::calibrate_reference(i2c_master_dev_handle_t i2c_dev,float dt) {
    double sum_pitch = 0.0, sum_roll = 0.0;

    ESP_LOGI(TAG, "Reference Calibration, Stand Straight");

    for (int i = 0; i < REFRENCE_SAMPLES; i++){
        read(i2c_dev);
        update_orientation(&data, dt);

        sum_pitch += pitch; 
        sum_roll  += roll;

        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay between samples
    }

    reference_pitch = (float)(sum_pitch / REFRENCE_SAMPLES);
    reference_roll  = (float)(sum_roll / REFRENCE_SAMPLES);
    reference_set = true;

    ESP_LOGI(TAG, "Reference calibration complete.");
    ESP_LOGI(TAG, "Reference — Pitch: %.2f° | Roll: %.2f°", reference_pitch, reference_roll);

}

void MPU6050::get_posture_deviation(float &pitch_deviation, float &roll_deviaiton){
    if(!reference_set){
        ESP_LOGW(TAG, "Refrence not set. Call calibrate_reference() first.");
        pitch_deviation = 0.0f;
        roll_deviaiton = 0.0f;
        return;
    }

    pitch_deviation = pitch - reference_pitch;
    roll_deviaiton = roll - reference_roll;
}