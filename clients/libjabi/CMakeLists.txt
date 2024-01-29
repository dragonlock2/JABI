cmake_minimum_required(VERSION 3.20.0)

add_library(libjabi
    interfaces/usb.cpp
    interfaces/uart.cpp
    peripherals/metadata.cpp
    peripherals/can.cpp
    peripherals/i2c.cpp
    peripherals/gpio.cpp
    peripherals/pwm.cpp
    peripherals/adc.cpp
    peripherals/dac.cpp
    peripherals/spi.cpp
    peripherals/uart.cpp
    peripherals/lin.cpp
)

if(MSVC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
    target_link_libraries(libjabi PkgConfig::libusb)
else()
    find_package(libusb CONFIG QUIET)
    if(NOT ${libusb_FOUND})
        find_library(LIBUSB_LIBRARIES NAMES usb-1.0)
        find_path(LIBUSB_INCLUDE_DIRS NAMES libusb.h PATH_SUFFIXES libusb-1.0)
    endif()
    target_link_libraries(libjabi pthread ${LIBUSB_LIBRARIES})
endif()

target_include_directories(libjabi PUBLIC
    ${LIBUSB_INCLUDE_DIRS}
    ..
    ../../include
)
