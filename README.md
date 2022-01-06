# JABI

JABI (Just Another Bridge Interface) makes creating and deploying USB bridge devices simple. With the firmware based on Zephyr RTOS, it is intended to be usable on virtually any microcontroller. The client software is also cross-platform.

## Disclaimer

This project is in the very early stages and much can change. It is very much the case of a solution looking for a problem. Half of the motivation for this project is to make significant progress in the long list of things I want to learn.

The major inspiration for this project was STLINK-V3-BRIDGE which was nice but I didn't like that the firmware was closed-source and tied to one vendor.

## TODO

This section documents the intended features of this project.

### Firmware

Zephyr appears to be the best choice for writing software that spans multiple microcontrollers. Mbed OS and FreeRTOS are close seconds.

- Written in C
- Run MCUboot w/ USB DFU
    - Redirect console to USB for debugging firmware dev
    - Actually log things properly don't just use `printf`
- Adding new board means just adding some DTS entries
- Fail *gracefully* if peripheral not supported

### Connectivity

The main focus is on USB, but I understand that in industry working with multiple USB devices can be a pain so may look into alternatives. One possible fix is adding a customizable ID to each device for easy detection.

- USB
- Ethernet
- WiFi
- BLE

### Supported Peripherals

This is a list of peripherals that I would find useful to have a programmatic way of using without having to buy an expensive tool or getting a dev board.

- CAN FD
- LIN
- SPI
- I2C
- UART
- GPIO
- ADC
- DAC

### Clients

- Standard interface written in Rust using libusb
    - Provide simplified interface to peripherals, sacrificing flexibility for ease of use
    - Allow for selecting which device is to be used in case multiple are connected
    - Very much the same idea as STLINK-V3-BRIDGE
- gRPC client/server bridging the standard interface to a network
- iOS app in React Native or Swift interacting with the gRPC server
- Android app in React Native or Kotlin interacting with the gRPC server
    - Potentially use the USB port
- QT6 cross-platform app interacting with gRPC server
- Website using WebUSB and Svelte JS for deployment on Chrome OS
    - Potentially use gRPC server as well

### Example Boards

To demonstrate how easy it is to use, deploy on various boards I've designed.

- NOGUSB
- K66F Breakout

### Extra

Fun things to look into but unlikely to actually reach

- CAN DBC
