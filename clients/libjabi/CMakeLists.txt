cmake_minimum_required(VERSION 3.20.0)

add_library(libjabi
    interfaces/usb.cpp
    interfaces/uart.cpp
    peripherals/metadata.cpp
    peripherals/can.cpp
    peripherals/i2c.cpp
)

find_package(libusb CONFIG QUIET)
if(NOT ${libusb_FOUND})
    find_library(LIBUSB_LIBRARIES NAMES usb-1.0)
    find_path(LIBUSB_INCLUDE_DIRS NAMES libusb.h PATH_SUFFIXES libusb-1.0)
endif()

target_link_libraries(libjabi ${LIBUSB_LIBRARIES})

target_include_directories(libjabi PUBLIC
    ${LIBUSB_INCLUDE_DIRS}
    ..
    ../../include
)
