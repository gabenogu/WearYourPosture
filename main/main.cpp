#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr gpio_num_t LED_PIN = GPIO_NUM_2;
constexpr uint32_t HIGH = 1;
constexpr uint32_t LOW = 0;

extern "C" void app_main(void){
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
