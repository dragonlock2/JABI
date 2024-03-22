cmake_minimum_required(VERSION 3.20.0)

add_library(jabi
    libjabi/interfaces/usb.cpp
    libjabi/interfaces/uart.cpp
    libjabi/peripherals/metadata.cpp
    libjabi/peripherals/can.cpp
    libjabi/peripherals/i2c.cpp
    libjabi/peripherals/gpio.cpp
    libjabi/peripherals/pwm.cpp
    libjabi/peripherals/adc.cpp
    libjabi/peripherals/dac.cpp
    libjabi/peripherals/spi.cpp
    libjabi/peripherals/uart.cpp
    libjabi/peripherals/lin.cpp
)

if(MSVC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
    target_link_libraries(jabi PkgConfig::libusb)
else()
    find_package(libusb CONFIG QUIET)
    if(NOT ${libusb_FOUND})
        find_library(LIBUSB_LIBRARIES NAMES usb-1.0)
        find_path(LIBUSB_INCLUDE_DIRS NAMES libusb.h PATH_SUFFIXES libusb-1.0)
    endif()
    target_link_libraries(jabi pthread ${LIBUSB_LIBRARIES})
endif()

target_include_directories(jabi PUBLIC
    ${LIBUSB_INCLUDE_DIRS}
    .
    ../../include
)
