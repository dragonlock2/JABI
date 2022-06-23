# JABI

JABI (Just Another Bridge Interface) makes creating and deploying bridge devices that provide RPC to common microcontroller peripherals simple.

## Setup

### Firmware

Follow Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) to get dependencies installed. Then create a new workspace using this repo. We'll create it under `jabi`, but you can use whatever you want.

```
west init -m https://github.com/dragonlock2/JABI.git jabi
cd jabi && west update
```

To port any board, you'll need the following. See [firmware/boards](firmware/boards/) for examples.

- Zephyr board definition. See Zephyr's [Board Porting Guide](https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html) for help.
- `firmware/boards/<board>.conf` - enable device drivers and any desired settings
- `firmware/boards/<board>.overlay` - selects available interfaces and peripherals

Now compile and flash the firmware.

```
west build -b <board>
west flash
```

### Clients

TODO

## Architecture

### Interfaces

Interfaces are the available methods by which a client may connect. Multiple interfaces running concurrently is supported. The following interfaces are currently supported.

- USB
- UART

### Peripherals

Microcontroller peripherals are made available over each interface via a custom basic RPC. Each interface listens for request packets and dispatches them to the appropriate peripheral. The following peripherals are currently supported.

- Metadata

### Clients

Clients connect to the microcontroller over any one of the interfaces. The current clients are supported.

- `cpplib` - C++ library

TODO make library custom func w/ header
TODO add libusb to library and test
TODO include the firmware headers move if needed
TODO send raw packets over each interface
TODO headers with func defs in library
TODO test Linux and Windows

## TODO

The following clients.

- C++ library using libusb for USB (like STLINK-V3-BRIDGE)
- Python library using pybind11 to C++
    - make it `pip` locally installable
- gRPC client/server bridging C++ to a network
    - iOS app in React Native or Swift
    - Android app in React Native or Kotlin (USB too?)
    - QT6 cross-platform app
    - Website using Svelte JS (WebUSB too?)

The following peripherals.

- CAN FD
    - allow one filter bc USB FS is slow
- LIN
- SPI
- I2C
- UART
- GPIO
- ADC
- DAC

Fun things to look into one day.

- CAN DBC
- DFU beyond USB for MCUboot
- Alternative functions for pins
- gRPC server on the microcontroller
    - may need to convert USB to a serial port or ethernet gadget
- Ethernet support
    - Listen for TCP on port 42069 and dispatch to a thread pool, refuse if all used
- BLE and WiFi support (need to make a dev board)
- On-device web server
