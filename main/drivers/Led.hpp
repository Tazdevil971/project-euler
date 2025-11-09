#pragma once

#include <driver/gpio.h>

namespace euler {

class Led {
public:
    Led() {}
    bool init(gpio_num_t gpio);

    void on();
    void off();

private:
    bool is_init = false;
    gpio_num_t gpio = GPIO_NUM_NC;
};

}