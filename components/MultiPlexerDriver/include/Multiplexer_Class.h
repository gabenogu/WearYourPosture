#pragma once
#include <driver/i2c_master.h>
#include <cstdint>
#include <hal/gpio_types.h>
#include "mpu_6050.h"
#include <freertos/FreeRTOS.h>
#include <esp_log.h>
constexpr uint8_t MPLEXER_ADDRESS = 0x70;
constexpr int MAX_ACCELEROMETERS = 3;


/*
    This initializes the multiplexer(i2c device)
        - creates it as master device
        - will link to the 3 slave accelerometer devices
        - Will send a command to each device once per cycle and communicate with it
    One instance of one multiplexer device. Within each accelerometer class the task will be created to communicate with it.
*/
class Multiplexer : public MPU6050{
    private:
        static i2c_master_bus_handle_t bus_handle;
        static i2c_master_dev_handle_t dev_handle;
        static i2c_master_dev_handle_t accel_handles;
        MPU6050 accelerometers[3]; // will become 3 when full build
        void select_channel(uint8_t set_bits);
        void start_accels();
        void calculate_accel_combined(float & combined_x, float &combined_y, float &combined_z);
        void calculate_gyro_combined(float & combined_x, float &combined_y, float &combined_z);

    public:
        explicit Multiplexer();
        void init();

        static void multiplexer_task_thread(void *args);

};