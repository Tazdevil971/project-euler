#include "Bno08x.hpp"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include <utils/Defer.hpp>

static const char *TAG = "Bno08x";

using namespace euler;

Bno08x::Bno08x() : events{xEventGroupCreate()} { assert(events != nullptr); }

bool Bno08x::init(i2c_master_bus_handle_t bus, gpio_num_t intr,
                  gpio_num_t reset, gpio_num_t bootn) {
    if (is_init) return false;

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
    assert(gpio_isr_handler_add(intr, on_irq, events) == ESP_OK);

    i2c_device_config_t dev_config = {};
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = ADDRESS;
    dev_config.scl_speed_hz = 100'000;
    dev_config.scl_wait_us = 0;
    dev_config.flags.disable_ack_check = false;
    assert(i2c_master_bus_add_device(bus, &dev_config, &dev_handle) == ESP_OK);

    // Set the device in reset
    gpio_set_level(bootn, 0);
    gpio_set_level(reset, 0);

    // Signal the bus as free
    xEventGroupSetBits(events, FREE_EV);

    // Start up the service handler
    service.start("Bno08xService", 16 * 1024, 0, [this]() { service_func(); });

    this->intr = intr;
    this->reset = reset;
    this->bootn = bootn;

    is_init = true;
    return true;
}

bool Bno08x::start() {
    if (!is_init) return false;

    acquire_bus();

    // Start up the device
    gpio_set_level(bootn, 1);
    gpio_set_level(reset, 1);

    // Wait for a "reset complete" message
    while (1) {
        recv(portMAX_DELAY, false);

        if (header_in.chan == bno08x::channels::EXECUTABLE &&
            header_in.len == 5 && cargo_in[4] == 1) {
            break;
        } else {
            handle_generic();
        }
    }

    release_bus();
    return true;
}

bool Bno08x::enable_arvr_stabilized_rotation_vector(uint32_t report_interval) {
    if (!is_init) return false;

    acquire_bus();

    std::array<uint8_t, bno08x::SetFeatureCommand::SIZE> buf;

    bno08x::Header header{
        .len = buf.size(),
        .chan = bno08x::channels::SH2_CONTROL,
        .seq = channels[bno08x::channels::SH2_CONTROL].seq_num_out++};

    bno08x::SetFeatureCommand command{
        .feature_report_id = bno08x::report_id::ARVR_STABILIZED_ROTATION_VECTOR,
        .wake_up_enable = false,
        .always_on_enable = false,
        .report_interval = report_interval,
        .batch_interval = 0,
        .config_word = 0};

    bno08x::SetFeatureCommand::write(header, command, buf);

    send_raw(buf);

    while (1) {
        recv(portMAX_DELAY, false);

        if (header_in.chan == bno08x::channels::SH2_CONTROL &&
            header_in.len > 4 &&
            cargo_in[4] == bno08x::report_id::GET_FEATURE_RESPONSE) {
            break;
        } else {
            handle_generic();
        }
    }

    release_bus();
    return true;
}

void Bno08x::service_func() {
    while (1) {
        // Receive the data and acquire the bus
        if (!recv(portMAX_DELAY, true)) {
            // We release the bus anyway, even if we didn't acquire it, it's
            // safe
            release_bus();

            ESP_LOGE(TAG, "Unexpected internal error while receiving");
            // TODO: Should we reset the device if too many failures occur?
            continue;
        }

        // Dispatch the event
        handle_generic();

        // Release the bus for other functions
        release_bus();
    }
}

void Bno08x::handle_generic() {
    if (header_in.chan == bno08x::channels::INPUT_SENSOR_REPORTS ||
        header_in.chan == bno08x::channels::WAKE_INPUT_SENSOR_REPORTS) {
        // Process sensor data
        size_t off = 4;
        while ((header_in.len - off) >= 1) {
            if (cargo_in[off] == bno08x::report_id::BASE_TIMESTAMP) {
                // Base timestamp packet
                if ((header_in.len - off) <
                    bno08x::BaseTimestampReference::SIZE) {
                    ESP_LOGE(TAG,
                             "Invalid size for BaseTimestampReference packet");
                    return;
                }

                auto packet = bno08x::BaseTimestampReference::read(
                    {cargo_in.begin() + off, cargo_in.end()});
                ESP_LOGI(TAG, "Received BaseTimestampReference, base_delta: %lu",
                         packet.base_delta);

                off += bno08x::BaseTimestampReference::SIZE;
            } else if (cargo_in[off] == bno08x::report_id::TIMESTAMP_REBASE) {
                // Rebase timestamp packet
                if ((header_in.len - off) <
                    bno08x::RebaseTimestampReference::SIZE) {
                    ESP_LOGE(
                        TAG,
                        "Invalid size for RebaseTimestampReference packet");
                    return;
                }

                
                auto packet = bno08x::RebaseTimestampReference::read(
                    {cargo_in.begin() + off, cargo_in.end()});
                    ESP_LOGI(TAG,
                        "Received RebaseTimestampReference, rebase_delta: %lu",
                        packet.rebase_delta);
            
                        off += bno08x::RebaseTimestampReference::SIZE;
            } else if (cargo_in[off] ==
                       bno08x::report_id::ARVR_STABILIZED_ROTATION_VECTOR) {
                // Rebase timestamp packet
                if ((header_in.len - off) <
                    bno08x::ARVRStabilizedRotationVector::SIZE) {
                    ESP_LOGE(
                        TAG,
                        "Invalid size for ARVRStabilizedRotationVector packet");
                    return;
                }

                
                auto packet = bno08x::ARVRStabilizedRotationVector::read(
                    {cargo_in.begin() + off, cargo_in.end()});
                    ESP_LOGI(TAG,
                        "Received ARVRStabilizedRotationVector, x: %f, y: %f, "
                        "z: %f, w: %f",
                        packet.x, packet.y, packet.z, packet.w);
                        off += bno08x::ARVRStabilizedRotationVector::SIZE;
            } else {
                ESP_LOGW(TAG, "Bruh: %d", cargo_in[off]);
                return;
            }
        }
    }

    if (header_in.chan == bno08x::channels::SH2_CONTROL &&
        header_in.len == 20 &&
        cargo_in[4] == bno08x::report_id::COMMAND_RESPONSE) {
        // This is a command response
        ESP_LOGI(TAG, "Received a cargo on chan %d, len: %d, command id: %x",
                 header_in.chan, header_in.len, cargo_in[6] & 0x7f);
    } else if ((header_in.chan == bno08x::channels::SH2_CONTROL ||
                header_in.chan == bno08x::channels::INPUT_SENSOR_REPORTS ||
                header_in.chan ==
                    bno08x::channels::WAKE_INPUT_SENSOR_REPORTS) &&
               header_in.len >= 5) {
        // This is a generic SH2 control message
        ESP_LOGI(TAG, "Received a cargo on chan %d, len: %d, report id: %x",
                 header_in.chan, header_in.len, cargo_in[4]);
    } else {
        // This is a generic command
        ESP_LOGI(TAG, "Received a cargo on chan %d, len: %d", header_in.chan,
                 header_in.len);
    }
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

bool Bno08x::recv(TickType_t timeout, bool acquire_bus) {
    TimeOut_t timer;
    vTaskSetTimeOutState(&timer);

    // Read out the header
    std::array<uint8_t, 4> header;
    if (!recv_raw(header, timeout, acquire_bus)) return false;

    // Decode the header
    header_in = bno08x::Header::read(header);

    // Catch initial errors
    if (header_in.len & 0x8000) {
        ESP_LOGE(TAG, "SHTP packet continuation is not supported");
        return false;
    }

    if (header_in.len < 4) {
        ESP_LOGE(TAG, "SHTP packet too small");
        return false;
    }

    if (header_in.len > cargo_in.size()) {
        ESP_LOGE(TAG, "SHTP packet too big");
        return false;
    }

    // This is not technically an error, we can recover from this
    if (header_in.chan >= channels.size()) {
        ESP_LOGW(TAG, "SHTP channel too big: %d", header_in.chan);
    } else {
        // We can no do channel related stuff
        if (channels[header_in.chan].seq_num_in != header_in.seq)
            ESP_LOGW(TAG, "SHTP sequence number invalid");

        channels[header_in.chan].seq_num_in = header_in.seq + 2;
    }

    // Read rest of the packet
    xTaskCheckForTimeOut(&timer, &timeout);
    if (!recv_raw({cargo_in.begin(), header_in.len}, timeout, false))
        return false;

    return true;
}

void Bno08x::acquire_bus() {
    xEventGroupWaitBits(events, FREE_EV, pdTRUE, pdTRUE, portMAX_DELAY);
}

void Bno08x::release_bus() { xEventGroupSetBits(events, FREE_EV); }

bool Bno08x::send_raw(std::span<const uint8_t> buf) {
    esp_err_t err = i2c_master_transmit(dev_handle, buf.data(), buf.size(), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write to I2C with err: %d", err);
        return false;
    }

    return true;
}

bool Bno08x::recv_raw(std::span<uint8_t> buf, TickType_t timeout,
                      bool acquire_bus) {
    if (!wait_for_irq(timeout, acquire_bus)) return false;

    esp_err_t err = i2c_master_receive(dev_handle, buf.data(), buf.size(), -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from I2C with err: %d", err);
        return false;
    }

    return true;
}

bool Bno08x::wait_for_irq(TickType_t timeout, bool acquire_bus) {
    if (!is_init) return false;

    TimeOut_t timer;
    vTaskSetTimeOutState(&timer);

    // Check if the signal is already low
    while (1) {
        // Wait for the interrupt
        EventBits_t bits =
            xEventGroupWaitBits(events, acquire_bus ? IRQ_EV | FREE_EV : IRQ_EV,
                                pdTRUE, pdTRUE, timeout);

        if (gpio_get_level(intr) == 0) {
            return true;
        }

        if (xTaskCheckForTimeOut(&timer, &timeout) == pdTRUE) {
            return false;
        }

        // If we successfully acquired the bus, just release it
        if (acquire_bus && (bits & FREE_EV) != 0) {
            xEventGroupSetBits(events, FREE_EV);
        }
    }
}

void Bno08x::on_irq(void *that) {
    EventGroupHandle_t *events = reinterpret_cast<EventGroupHandle_t *>(that);
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (xEventGroupSetBitsFromISR(events, IRQ_EV,
                                  &higher_priority_task_woken) != pdFAIL) {
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}