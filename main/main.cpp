#include "Euler.hpp"

#include <memory>

extern "C" void app_main() {
    std::unique_ptr<euler::Euler> euler = std::make_unique<euler::Euler>();
    euler->init();
    euler->main();
}