#pragma once

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <utils/Tasklet.hpp>

#include <array>
#include <span>

#include "Bno08xProto.hpp"

namespace euler {

class Bno08x {
public:
    Bno08x();
    bool init(i2c_master_bus_handle_t bus, gpio_num_t intr, gpio_num_t reset,
              gpio_num_t bootn);

    bool start();
    bool enable_arvr_stabilized_rotation_vector(uint32_t report_interval);

private:
    void service_func();

    void handle_generic();

    const char* device_error_to_str(uint8_t code);

    bool recv(TickType_t timeout, bool acquire_bus);

    void acquire_bus();
    void release_bus();

    bool send_raw(std::span<const uint8_t> buf);
    bool recv_raw(std::span<uint8_t> buf, TickType_t timeout, bool acquire_bus);
    bool wait_for_irq(TickType_t timeout, bool acquire_bus);

    static void on_irq(void* that);

    static constexpr uint8_t ADDRESS = 0x4a;
    static constexpr uint8_t CHANNEL_NUM = 6;

    bool is_init = false;
    i2c_master_dev_handle_t dev_handle = nullptr;
    gpio_num_t bootn = GPIO_NUM_NC;
    gpio_num_t intr = GPIO_NUM_NC;
    gpio_num_t reset = GPIO_NUM_NC;

    static constexpr EventBits_t IRQ_EV = 1 << 0;
    static constexpr EventBits_t FREE_EV = 1 << 1;
    EventGroupHandle_t events = nullptr;

    Tasklet service;

    struct ChannelInfo {
        uint8_t seq_num_in = 0;
        uint8_t seq_num_out = 0;
    };

    std::array<ChannelInfo, CHANNEL_NUM> channels;

    bno08x::Header header_in;
    std::array<uint8_t, 1024> cargo_in;
};

}  // namespace euler