idf_component_register(
    SRCS 
        main.cpp
        Euler.cpp
        drivers/Bno08x.cpp
        drivers/Led.cpp
    REQUIRES
        spi_flash
        esp_driver_gpio
        esp_driver_i2c
    INCLUDE_DIRS ".")