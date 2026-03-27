#include "include/Multiplexer_Class.h"
#define PORT_NUM I2C_NUM_0
#define MAX_ACCEL 2     // will change to 3 when time for final build
constexpr Addresses addr[] = {Addresses::ADDRESS_ONE_MP, Addresses::ADDRESS_TWO_MP};
i2c_master_dev_handle_t Multiplexer::dev_handle = {0};
i2c_master_bus_handle_t Multiplexer::bus_handle = {0};


Multiplexer::Multiplexer():
    accelerometers{
        MPU6050(static_cast<uint8_t>(Addresses::ADDRESS_ONE_MP)), 
        MPU6050(static_cast<uint8_t>(Addresses::ADDRESS_TWO_MP))
    }
{
    
}

void Multiplexer::init(){
    const i2c_master_bus_config_t master_conf = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .intr_priority = 0,
        .flags = {.enable_internal_pullup = 1}
    };

    i2c_new_master_bus(&master_conf,&bus_handle);

    const i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = (uint16_t)MPLEXER_ADDRESS,
        .scl_speed_hz = (uint32_t)100000,
        .scl_wait_us = 0,
        .flags = {.disable_ack_check = 0}

    };
    i2c_master_bus_add_device(bus_handle,&dev_config,&dev_handle);
    
}

void Multiplexer::multiplexer_task_thread(void * args){
    Multiplexer * arg = static_cast<Multiplexer*>(args);

    const i2c_device_config_t accel_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x68,
        .scl_speed_hz = 400000
    };
    // Dev handles for both accelerometers
    i2c_master_dev_handle_t accel_dev_handle;

    i2c_master_bus_add_device(bus_handle,&accel_config,&accel_dev_handle);

    // for(int i = 0; i < MAX_ACCEL; i++){
    //     arg->select_channel(i);
    //     vTaskDelay(2/portTICK_PERIOD_MS);
    //     arg->accelerometers[i].init_accel(accel_dev_handle);
    // }
    arg->select_channel(0);
    arg->accelerometers[0].init_accel(accel_dev_handle);

    while(1){
        // for(int i = 0; i < MAX_ACCEL; i++){
        //     arg->select_channel(i);
        //     printf("Printing Accel Data idx = %d\n", i);
        //     vTaskDelay(2/portTICK_PERIOD_MS);
        //     arg->accelerometers[i].read(accel_dev_handle);
        //     vTaskDelay(ONE_TIME_DELAY);
        // }
        arg->select_channel(0);
        vTaskDelay(2/portTICK_PERIOD_MS);
        arg->accelerometers[0].read(accel_dev_handle);
        vTaskDelay(ONE_TIME_DELAY);
    }
}

void Multiplexer::select_channel(int index){
    uint8_t cmd = 0b0000'0001 << index+1;
    i2c_master_transmit(dev_handle,&cmd,sizeof(cmd),ONE_TIME_DELAY);
}
