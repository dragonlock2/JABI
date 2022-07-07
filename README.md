# JABI

JABI (Just Another Bridge Interface) makes creating and deploying bridge devices that provide RPC to common microcontroller peripherals simple.

## Architecture

Interfaces are the available methods by which a single client may connect. Multiple interfaces running concurrently is supported. The following interfaces are currently supported.

- USB
- UART

Microcontroller peripherals are made available over each interface via a custom basic RPC. Each interface listens for request packets and dispatches them to the appropriate peripheral. Multiple instances of each peripheral type is supported. The following peripherals are currently supported.

- Metadata
- CAN FD
- SPI controller
- I2C controller
- UART
- GPIO
- PWM
- ADC
- DAC

Clients connect to the microcontroller over any one of the interfaces. The following clients are supported.

- `libjabi` - C++ library
- `pyjabi` - Python library
- `grpc-server` - gRPC server

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

First let's install a few dependencies.

- macOS
    - `brew install git cmake libusb grpc openssl`
- Linux
    - `apt install git cmake libusb-1.0-0-dev libssl-dev`
    - Install [grpc](https://github.com/grpc/grpc/blob/master/BUILDING.md) from source.
- Windows
    - Install [Visual Studio C++](https://visualstudio.microsoft.com/vs/features/cplusplus/) for its C++ compiler. It's also an IDE.
    - Use [winget](https://docs.microsoft.com/en-us/windows/package-manager/winget/) to install [git](https://winget.run/pkg/Git/Git) and [CMake](https://winget.run/pkg/Kitware/CMake).
    - Use [vcpkg](https://github.com/microsoft/vcpkg) to install [libusb](https://vcpkg.info/port/libusb), [gRPC](https://vcpkg.info/port/grpc), [OpenSSL](https://vcpkg.info/port/openssl), and [getopt](https://vcpkg.info/port/getopt).
    - Use [Zadig](https://zadig.akeo.ie) to install the [WinUSB](https://github.com/libusb/libusb/wiki/Windows#driver-installation) driver on any `JABI USB` devices.

We use CMake for our build system which has the following standard build process. For macOS gRPC projects, you may need to do `cmake .. -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)` instead.

```
cd <project path>
mkdir build && cd build
cmake ..
cmake --build .
```

#### libjabi

[`libjabi`](clients/libjabi) is provided as a CMake library and can be added to any CMake project using `add_subdirectory`. An example project is in [examples/libjabi](examples/libjabi).

#### pyjabi

[`pyjabi`](clients/pyjabi) can be installed from a local directory using `pip` for easy management. An example using it is in [examples/pyjabi](examples/pyjabi).

```
pip install clients/pyjabi
```

#### grpc-server

[`grpc-server`](clients/grpc-server) bridges one device to a network and can handle parallel requests. It provides various arguments for selecting the desired device. An example client is in [examples/grpc-client](examples/grpc-client).

## Known Issues

- UART interface timeout doesn't work when binded in Python.

## TODO

The following gRPC clients.

- iOS app in React Native or Swift
- Android app in React Native or Kotlin (USB too?)
- QT6 cross-platform app
- Website using Svelte JS (WebUSB too?)

Fun things to look into one day.

- Alternative functions for pins
- Improve security and robustness (encryption? CRC? gRPC SSL?)
- Proper RPC server on the microcontroller (gRPC? Thrift?)
- Multiple USB interfaces for multiple clients (raw USB is \~25% faster than CDC-ACM)
- Ethernet support - thread pool from CS162!
- BLE and WiFi support (need to make a dev board)
- USB Linux drivers to show up under `/dev`
- USB HS dev board for comparable performance to STLINK-V3
