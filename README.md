# JABI

JABI (Just Another Bridge Interface) makes creating and deploying bridge devices that provide RPC to common microcontroller peripherals simple. Zephyr RTOS makes porting to any board straigntforward. Various clients enable cross-platform usage.

## Setup

Follow Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) to get dependencies installed. Then create a new workspace using this repo. We'll create it under `jabi`, but you can use whatever you want.

```
west init -m https://github.com/dragonlock2/JABI.git jabi
cd jabi && west update
```

## TODO

Everything here is very much subject to change.

### Clients

Clients connect to the microcontroller over one of a number of supported interfaces.

- C++ library using libusb for USB (like STLINK-V3-BRIDGE)
- Python library using pybind11 to C++
- gRPC client/server bridging C++ to a network
    - iOS app in React Native or Swift
    - Android app in React Native or Kotlin (USB too?)
    - QT6 cross-platform app
    - Website using Svelte JS (WebUSB too?)

### Firmware

Porting to any board should just involve adding a DTS overlay that selects the appropriate peripherals and interface.

Implementation consists of a custom basic RPC with a single client. Accessing unavailable peripherals should return an error code.

### Interface

A thin compatibility layer parses out RPC packets from the selected interface to dispatch to peripherals.

- USB
- UART
- BLE
- Ethernet
- WiFi

### Peripherals

Peripherals match the RPC payload to a function prototype which parses out the arguments. Multiple instances of each peripheral type are supported.

- Metadata
    - serial number to make distinguishing devices easier
- CAN FD
    - allow one filter bc USB FS is slow
- LIN
- SPI
- I2C
- UART
- GPIO
- ADC
- DAC

### Extra

Fun things to look into but unlikely to actually reach

- CAN DBC
- DFU beyond USB for MCUboot
- Alternative functions for pins
