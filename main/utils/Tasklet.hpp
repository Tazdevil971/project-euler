#pragma once

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include <functional>

namespace euler {

class Tasklet {
public:
    Tasklet() {}
    Tasklet(const Tasklet &) = delete;
    Tasklet(Tasklet &&) = delete;

    template <typename F>
        requires std::is_invocable_r_v<void, F>
    bool start(const char *name, uint32_t stack, UBaseType_t priority, F func) {
        if (task != nullptr) return false;

        this->func = func;
        if (xTaskCreate(task_fn, name, stack, this, priority, &task) !=
            pdPASS) {
            ESP_LOGE("Tasklet", "Failed to create a new task");
            return false;
        }

        return true;
    }

protected:
    static void task_fn(void *arg) {
        std::invoke(reinterpret_cast<Tasklet *>(arg)->func);
    }

    TaskHandle_t task = nullptr;

private:
    std::function<void()> func;
};

}  // namespace euler