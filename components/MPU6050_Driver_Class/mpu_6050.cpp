#include <stdio.h>
#include "mpu_6050.h"
#include "esp_err.h"

static constexpr uint8_t MUX_ADDR = 0x70; //change depending on multiplexer

static esp_err_t mux_select_channel(i2c_port_t port, uint8_t channel){
    if (channel > 7 ) return ESP_ERR_INVALID_ARG;
    uint8_t control = (1 << channel);
    return i2c_master_write_to_device(
        port, 
        MUX_ADDR,
        &control,
        1,
        1000 / portTICK_PERIOD_MS
    );
}

extern "C" void app_main(void){
    MPU6050 sensor(I2C_NUM_0, 0x68);

    while (true){
        if (mux_select_channel(I2C_NUM_0, 0) == ESP_OK){
            print_data(sensor);
        } else {
            printf("Failed to select mux channel\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

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
    i2c_master_write_to_device(i2c_port, slave_addr, wake_cmd, sizeof(wake_cmd), 1000 / portTICK_PERIOD_MS); 
    const uint8_t config_register[2] = {0x1C,0x00};
    i2c_master_write_to_device(i2c_port, slave_addr, config_register, sizeof(config_register),1000 / portTICK_PERIOD_MS);
}

inline int16_t combine(uint8_t high, uint8_t low) {
    return (int16_t)((high << 8) | low);
}

esp_err_t MPU6050::init() {

   
    // 1. Setup and install the I2C driver (i2c_param_config, etc.)

    i2c_config_t conf = {};
     conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_21; 
    conf.scl_io_num = GPIO_NUM_22; 
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(i2c_port, &conf);
    i2c_driver_install(i2c_port, conf.mode, 0, 0,0);
    
     // 2. Send the wake-up command
    uint8_t wake_cmd[2] = {WAKEUP_REG, 0x00};
    esp_err_t err = i2c_master_write_to_device(i2c_port, slave_addr, wake_cmd, sizeof(wake_cmd), 1000 / portTICK_PERIOD_MS);//time period to wait for a command.
    
    if (err != ESP_OK) {
        return err;
    }   
    // 3. Send the config command
    const uint8_t config_register[2] = {0x1C,0x00};    
    // 4. Return the status
    esp_err_t err = i2c_master_write_to_device(i2c_port, slave_addr, wake_cmd, sizeof(wake_cmd), 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        return err;
    }
    return ESP_OK;
}


void MPU6050:: read(SensorData *data) {
    // Read 14 bytes of data starting from the DATA_START_REG
    uint8_t buffer[14];
    uint8_t reg = DATA_START_REG;
    i2c_master_write_read_device(i2c_port,slave_addr, &reg, 1, buffer, 14, 1000 / portTICK_PERIOD_MS);
    
    // Convert the raw data to signed integers
    data->accel.x = combine(buffer[0], buffer[1]) / 16384.0f;
    data->accel.y = combine(buffer[2], buffer[3])/16384.0f;
    data->accel.z = combine(buffer[4], buffer[5]) / 16384.0f;

    data->temp = combine(buffer[6], buffer[7]) / 340.0f + 36.53f;

    data->gyro.x = combine(buffer[8], buffer[9]) / 131.0f;
    data->gyro.y = combine(buffer[10], buffer[11]) / 131.0f;
    data->gyro.z = combine(buffer[12], buffer[13]) / 131.0f;
}
void print_data(MPU6050 &sensor) {
    SensorData data;
    sensor.read(&data);
    printf("Accel: X=%f, Y=%f, Z=%f\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%f, Y=%f, Z=%f\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Temp: %f\n", data.temp);
}
