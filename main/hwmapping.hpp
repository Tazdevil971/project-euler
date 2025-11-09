#pragma once

#include <driver/gpio.h>
#include <driver/i2c_master.h>

namespace euler::hwmapping {

constexpr gpio_num_t USR_LED1 = GPIO_NUM_8;
constexpr gpio_num_t USR_LED2 = GPIO_NUM_10;

constexpr gpio_num_t USR_BTN1 = GPIO_NUM_5;
constexpr gpio_num_t USR_BTN2 = GPIO_NUM_11;

// constexpr adc_channel_t BAT_SENSE = ADC_CHANNEL_4;

constexpr gpio_num_t USB_SLEEP = GPIO_NUM_0;
constexpr gpio_num_t USB_PWREN = GPIO_NUM_1;
constexpr gpio_num_t USB_BCD = GPIO_NUM_2;
constexpr gpio_num_t USB_RXF = GPIO_NUM_15;
constexpr gpio_num_t USB_KEEP_AWAKE = GPIO_NUM_18;

constexpr gpio_num_t CHG_INT = GPIO_NUM_3;

constexpr gpio_num_t BNO_IRQ = GPIO_NUM_21;
constexpr gpio_num_t BNO_RESET = GPIO_NUM_22;
constexpr gpio_num_t BNO_BOOTN = GPIO_NUM_23;

constexpr gpio_num_t I2C_SDA = GPIO_NUM_19;
constexpr gpio_num_t I2C_SCL = GPIO_NUM_20;

constexpr gpio_num_t I2C_LP_SDA = GPIO_NUM_6;
constexpr gpio_num_t I2C_LP_SCL = GPIO_NUM_7;

}  // namespace euler::hwmapping
