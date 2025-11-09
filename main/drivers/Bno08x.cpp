#include "Bno08x.hpp"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include <utils/Defer.hpp>

static const char *TAG = "Bno08x";

using namespace euler;

Bno08x::Bno08x() : irq_ev{xEventGroupCreate()} { assert(irq_ev != nullptr); }

bool Bno08x::start(i2c_master_bus_handle_t bus, gpio_num_t intr,
                   gpio_num_t reset, gpio_num_t bootn) {
    if (is_start) return false;

    gpio_config_t reset_config = {};
    reset_config.pin_bit_mask = (1 << reset) | (1 << bootn);
    reset_config.mode = GPIO_MODE_OUTPUT;
    reset_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    reset_config.pull_up_en = GPIO_PULLUP_DISABLE;
    reset_config.intr_type = GPIO_INTR_DISABLE;
    assert(gpio_config(&reset_config) == ESP_OK);

    gpio_config_t intr_config = {};
    intr_config.pin_bit_mask = 1 << intr;
    intr_config.mode = GPIO_MODE_INPUT;
    intr_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // Keep this at pull-up to keep the line stable when the other device is not
    // actively driving the line
    intr_config.pull_up_en = GPIO_PULLUP_ENABLE;
    intr_config.intr_type = GPIO_INTR_NEGEDGE;
    assert(gpio_config(&intr_config) == ESP_OK);
    assert(gpio_isr_handler_add(intr, on_irq, irq_ev) == ESP_OK);

    i2c_device_config_t dev_config = {};
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = ADDRESS;
    dev_config.scl_speed_hz = 100'000;
    dev_config.scl_wait_us = 0;
    dev_config.flags.disable_ack_check = false;
    assert(i2c_master_bus_add_device(bus, &dev_config, &dev_handle) == ESP_OK);

    gpio_set_level(bootn, 0);
    gpio_set_level(reset, 0);

    // Start up the device
    gpio_set_level(bootn, 1);
    gpio_set_level(reset, 1);

    this->intr = intr;
    this->reset = reset;
    this->bootn = bootn;

    is_start = true;
    return true;
}

void Bno08x::foo() {
    // std::array<uint8_t, 6> bruh;
    // bruh[0] = 6;
    // bruh[1] = 0;
    // bruh[2] = 2;
    // bruh[3] = 0;
    // bruh[4] = 0xf9;
    // bruh[5] = 0;
    // send_raw(bruh);

    // xEventGroupWaitBits(irq_ev, 1, pdTRUE, pdTRUE, portMAX_DELAY);
    // printf("AAA: %ld\n", xEventGroupGetBits(irq_ev));

    recv(portMAX_DELAY);
}

void Bno08x::handle_chan0(std::span<const uint8_t> data) {
    // Channel 0 is dedicated to control
    if (data.size() < 1) {
        ESP_LOGW(TAG, "");
        return;
    }
}

void Bno08x::handle_advertisement(std::span<const uint8_t> data) {
    // TODO: If we really want to
}

void Bno08x::handle_error_list(std::span<const uint8_t> data) {
    for (size_t i = 0; i < data.size(); i++)
        ESP_LOGW(TAG, "Device sent error: %s (code: %d)",
                 device_error_to_str(data[i]), data[i]);
}

const char *Bno08x::device_error_to_str(uint8_t code) {
    switch (code) {
        case 0:
            return "<no error>";
        case 1:
            return "Hub application attempted to exceed maximum read cargo "
                   "length";
        case 2:
            return "Host write was too short (need at least a 4-byte header)";
        case 3:
            return "Host wrote a header with length greater than maximum write "
                   "cargo length";
        case 4:
            return "Host wrote a header with length less than or equal to "
                   "header length (either invalid or no payload). Note that a "
                   "length of 0 is permitted, indicating \"no cargo\"";
        case 5:
            return "Host wrote beginning of fragmented cargo (transfer length "
                   "was less than full cargo length), fragmentation not "
                   "supported";
        case 6:
            return "Host wrote continuation of fragmented cargo (continuation "
                   "bit sent), fragmentation not supported";
        case 7:
            return "Unrecognized command on control channel";
        case 8:
            return "Unrecognized parameter to get-advertisement command";
        case 9:
            return "Host wrote to unrecognized channel";
        case 10:
            return "Advertisement request received while Advertisement "
                   "Response was pending";
        case 11:
            return "Host performed a write operation before the hub had "
                   "finished sending its advertisement response";
        case 12:
            return "Error list too long to send, truncated";
        case 14:
            return "Invalid next sequence index";
        default:
            return "<unknown>";
    }
}

bool Bno08x::recv(TickType_t timeout) {
    // First read the header
    std::array<uint8_t, 4> header;
    if (!recv_raw(header, portMAX_DELAY)) return false;

    // Decode the header
    uint16_t len = (header[0] | header[1] << 8);

    // Catch initial errors
    if (len & 0x8000) {
        ESP_LOGE(TAG, "SHTP packet continuation is not supported");
        return false;
    }

    if (len < 4) {
        ESP_LOGE(TAG, "SHTP packet too small");
        return false;
    }

    if (len > rx_buf.size()) {
        ESP_LOGE(TAG, "SHTP packet too big");
        return false;
    }

    // Read rest of the packet
    if (!recv_raw({rx_buf.begin(), len}, portMAX_DELAY)) return false;

    ESP_LOGI(TAG, "Yeetus");

    return true;
}

bool Bno08x::send_raw(std::span<const uint8_t> data) {
    esp_err_t err =
        i2c_master_transmit(dev_handle, data.data(), data.size(), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to I2C with err: %d", err);
        return false;
    }

    return true;
}

bool Bno08x::recv_raw(std::span<uint8_t> data, TickType_t timeout) {
    if (!wait_for_irq(timeout)) return false;

    esp_err_t err =
        i2c_master_receive(dev_handle, data.data(), data.size(), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from I2C with err: %d", err);
        return false;
    }

    return true;
}

bool Bno08x::wait_for_irq(TickType_t timeout) {
    if (!is_start) return false;

    TimeOut_t timer;
    vTaskSetTimeOutState(&timer);

    // First clear the event group
    xEventGroupClearBits(irq_ev, 1);
    // Check if the signal is already low
    while (gpio_get_level(intr) != 0) {
        if (xTaskCheckForTimeOut(&timer, &timeout) == pdTRUE) {
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        // Wait for the interrupt
        xEventGroupWaitBits(irq_ev, 1, pdTRUE, pdTRUE, timeout);
    }

    return true;
}

void Bno08x::on_irq(void *that) {
    EventGroupHandle_t *irq_ev = reinterpret_cast<EventGroupHandle_t *>(that);
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (xEventGroupSetBitsFromISR(irq_ev, 1, &higher_priority_task_woken) !=
        pdFAIL) {
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}