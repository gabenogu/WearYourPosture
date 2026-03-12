#include "mpu_6050.h"


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
}



void MPU6050:: read(SensorData *data) {
    // Read 14 bytes of data starting from the DATA_START_REG
    uint8_t buffer[14];
    uint8_t reg = DATA_START_REG;
    i2c_master_write_read_device(i2c_port,slave_addr, &reg, 1, buffer, 14, 1000 / portTICK_PERIOD_MS);
    
    // Convert the raw data to signed integers
    data->accel.x = (buffer[0] << 8) | buffer[1];
    data->accel.y = (buffer[2] << 8) | buffer[3];
    data->accel.z = (buffer[4] << 8) | buffer[5];

    data->temp = (buffer[6] << 8) | buffer[7];

    data->gyro.x = (buffer[8] << 8) | buffer[9];
    data->gyro.y = (buffer[10] << 8) | buffer[11];
    data->gyro.z = (buffer[12] << 8) | buffer[13]; 
}
void print_data(MPU6050 &sensor) {
    SensorData data;
    sensor.read(&data);
    printf("Accel: X=%d, Y=%d, Z=%d\n", data.accel.x, data.accel.y, data.accel.z);
    printf("Gyro: X=%d, Y=%d, Z=%d\n", data.gyro.x, data.gyro.y, data.gyro.z);
    printf("Temp: %d\n", data.temp);
    
}