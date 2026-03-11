#pragma once

#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr gpio_num_t LED_PIN = GPIO_NUM_2;
constexpr uint32_t HIGH = 1;
constexpr uint32_t LOW = 0;

void set_led();