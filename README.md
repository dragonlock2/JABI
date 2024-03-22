# JABI

JABI (Just Another Bridge Interface) makes creating and deploying bridge devices that provide RPC to common microcontroller peripherals simple.

## Architecture

Interfaces are the available methods by which a single client may connect. Multiple interfaces running concurrently is supported. The following interfaces are currently supported.

- USB
- UART

Microcontroller peripherals are made available over each interface via a custom basic RPC. Each interface listens for request packets and dispatches them to the appropriate peripheral. Multiple instances of each peripheral type is supported. The following peripherals are currently supported.

- Metadata
- CAN (FD)
- LIN
- SPI controller
- I2C controller
- UART
- GPIO
- PWM
- ADC
- DAC

Clients connect to the microcontroller over any one of the interfaces. The following clients are supported.

- C++
- Python
- gRPC
- Rust

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

If you want to build from source, you'll need to install a few dependencies.

- macOS
    - `brew install git cmake libusb grpc openssl`
- Linux
    - `apt install git cmake libusb-1.0-0-dev libssl-dev`
    - Install [grpc](https://github.com/grpc/grpc/blob/master/BUILDING.md) from source.
- Windows
    - Install [Visual Studio C++](https://visualstudio.microsoft.com/vs/features/cplusplus/) for its C++ compiler.
    - Use [winget](https://docs.microsoft.com/en-us/windows/package-manager/winget/) to install [git](https://winget.run/pkg/Git/Git) and [CMake](https://winget.run/pkg/Kitware/CMake).
    - Use [vcpkg](https://github.com/microsoft/vcpkg) to install `libusb`, `gRPC`, `OpenSSL`, and `getopt`.

#### C++

C++ support is provided as a CMake library and can be added to any CMake project using `add_subdirectory`. An example project is in [examples/cpp](examples/cpp).

#### Python

A Python library is published on [PyPI](TODO). For the latest changes, it can be built and installed locally by running the following. An example using it is in [examples/python](examples/python).

```
pip install clients/python
```

#### gRPC

Protobuf definitions are located in [`jabi.proto`](include/protos/jabi.proto). [`grpc-server`](clients/grpc-server) is a reference server implementation that bridges one device to a network and can handle parallel requests. It provides various arguments for selecting the desired device. An example client is in [examples/grpc-client](examples/grpc-client).

#### Rust

A Rust crate is published on [crates.io](https://crates.io/crates/jabi). For the latest changes, it can be added locally. An example project is in [examples/rust](examples/rust).

## TODO

The following gRPC clients.

- Google Flutter cross-platform app

Fun things to look into one day.

- Better documentation...
- Unit testing...
- Alternative functions for pins
- Move to Thrift for RPC (natively supported in Zephyr!)
    - Network (Ethernet, WiFi) support
- BLE support
- USB Linux drivers to show up under `/dev`
- USB HS dev board for comparable performance to STLINK-V3
