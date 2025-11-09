#include "Euler.hpp"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include "hwmapping.hpp"

static const char* TAG = "Euler";

using namespace euler;

void Euler::init() {
    gpio_install_isr_service(0);

    // Init I2C bus
    i2c_master_bus_config_t i2c_config = {};
    i2c_config.i2c_port = I2C_NUM_0;
    i2c_config.sda_io_num = hwmapping::I2C_SDA;
    i2c_config.scl_io_num = hwmapping::I2C_SCL;
    i2c_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_config.glitch_ignore_cnt = 7;
    i2c_new_master_bus(&i2c_config, &i2c_handle);

    // Init LEDs
    if (!usr_led1.init(hwmapping::USR_LED1)) {
        ESP_LOGE(TAG, "Failed to init user led 1");
    }

    if (!usr_led2.init(hwmapping::USR_LED2)) {
        ESP_LOGE(TAG, "Failed to init user led 2");
    }

    // Init IMU
    if (!bno08x.start(i2c_handle, hwmapping::BNO_IRQ, hwmapping::BNO_RESET,
                      hwmapping::BNO_BOOTN)) {
        ESP_LOGE(TAG, "Failed to start bno08x");
    }
}

void Euler::main() {
    for (int i = 0; i < 10; i++)
        bno08x.foo();

    while (true) {
        usr_led1.on();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        usr_led1.off();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}