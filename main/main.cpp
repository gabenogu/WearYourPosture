#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_bt.h>

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

//initialize teh bluetooth controller to allocate task and other resources
esp_err_t esp_bt_controller_init(const esp_bt_controller_config_t *cfg){
    return ESP_OK, ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_ARG, ESP_ERR_NO_MEM;
}

esp_err_t esp_bt_controller_enable(esp_bt_mode_t mode){
    return ESP_OK, ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_ARG;
}

//De-initialize Bluetooth Controller to free resources and delete tasks.
esp_err_t esp_bt_controller_deinit(void){
    return ESP_OK, ESP_ERR_INVALID_STATE;
}