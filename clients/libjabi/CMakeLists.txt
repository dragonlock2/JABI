cmake_minimum_required(VERSION 3.20.0)

add_library(jabi
    interfaces/usb.cpp
    interfaces/uart.cpp
    peripherals/metadata.cpp
)

find_library(LIBUSB_LIBRARY NAMES usb-1.0)
find_path(LIBUSB_INCLUDE_DIR NAMES libusb-1.0/libusb.h)

target_link_libraries(jabi ${LIBUSB_LIBRARY})

target_include_directories(jabi PUBLIC
    ${LIBUSB_INCLUDE_DIR}
    ..
    ../../include
)
