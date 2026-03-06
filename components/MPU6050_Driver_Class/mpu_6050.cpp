#include "include/mpu_6050.h"
void set_led(){
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN,GPIO_MODE_OUTPUT);
    while(1){
        gpio_set_level(LED_PIN, HIGH);
        printf("LED ON\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_PIN,LOW);
        printf("LED OFF\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}