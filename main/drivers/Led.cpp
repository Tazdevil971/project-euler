#include "Led.hpp"

#include <esp_log.h>

static const char* TAG = "Led";

using namespace euler;

bool Led::init(gpio_num_t gpio) {
    if (is_init) return false;

    esp_err_t err;

    // Setup gpio
    gpio_config_t config = {};
    config.pin_bit_mask = 1 << gpio;
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    err = gpio_config(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED driver with err: %d", err);
        return false;
    }

    // Turn off the LED
    gpio_set_level(gpio, 1);

    this->gpio = gpio;

    is_init = true;
    return true;
}

void Led::on() {
    if (is_init) gpio_set_level(gpio, 0);
}

void Led::off() {
    if (is_init) gpio_set_level(gpio, 1);
}
