#pragma once
#include <driver/i2c_master.h>
#include <cstdint>
#include <hal/gpio_types.h>
#include "mpu_6050.h"
#include <freertos/FreeRTOS.h>
constexpr uint8_t MPLEXER_ADDRESS = 0x70;
enum class Addresses : uint8_t{
    ADDRESS_ONE_MP = 0x71,
    ADDRESS_TWO_MP = 0x72,
    ADDRESS_THREE_MP = 0x74,
    ADDRESS_FOUR_MP = 0x75,
    ADDRESS_SIX_MP = 0x76,
    ADDRESS_SEVEN_MP = 0x77,
};

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
        MPU6050 accelerometers[2]; // will become 3 when full build
        void select_channel(int index);
    public:
        explicit Multiplexer();
        void init();

        static void multiplexer_task_thread(void *args);

};