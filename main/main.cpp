#include "mpu_6050.h"
#include "Bluetooth_WYP.h"
#include <iostream>
#include "Multiplexer_Class.h"
#include <freertos/task.h>

extern "C" void app_main(void){
    static Multiplexer mp;
    mp.init();
    xTaskCreate(mp.multiplexer_task_thread,"multiplexer task", 4096,&mp,configMAX_PRIORITIES-1,NULL);
}
