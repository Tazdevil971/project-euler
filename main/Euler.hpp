#pragma once

#include <drivers/Led.hpp>
#include <drivers/Bno08x.hpp>

#include <driver/i2c_master.h>

namespace euler {

class Euler {
public:
    Euler() {}    

    void init();
    void main();

private:
    i2c_master_bus_handle_t i2c_handle = nullptr;

    Led usr_led1;
    Led usr_led2;
    Bno08x bno08x;
};

}