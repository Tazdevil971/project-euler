#pragma once

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>

#include <array>
#include <span>

namespace euler {

class Bno08x {
public:
    Bno08x();
    bool start(i2c_master_bus_handle_t bus, gpio_num_t intr, gpio_num_t reset,
               gpio_num_t bootn);

    void foo();

private:
    const char* device_error_to_str(uint8_t code);

    void handle_chan0(std::span<const uint8_t> data);
    void handle_error_list(std::span<const uint8_t> data);
    void handle_advertisement(std::span<const uint8_t> data);

    bool recv(TickType_t timeout);

    bool send_raw(std::span<const uint8_t> data);
    bool recv_raw(std::span<uint8_t> data, TickType_t timeout);
    bool wait_for_irq(TickType_t timeout);

    static void on_irq(void* that);

    static constexpr uint8_t ADDRESS = 0x4a;

    bool is_start = false;
    i2c_master_dev_handle_t dev_handle = nullptr;
    gpio_num_t bootn = GPIO_NUM_NC;
    gpio_num_t intr = GPIO_NUM_NC;
    gpio_num_t reset = GPIO_NUM_NC;

    EventGroupHandle_t irq_ev = nullptr;

    std::array<uint8_t, 1024> rx_buf;
};

}  // namespace euler