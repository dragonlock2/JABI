# JABI

JABI (Just Another Bridge Interface) makes creating and deploying bridge devices that provide RPC to common microcontroller peripherals simple.

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

- `libjabi` - C++ library
- `pyjabi` - Python library

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

First let's install dependencies. For macOS and Linux, it's as simple as the following.

```
brew install cmake ninja libusb # MacOS
apt install g++ cmake ninja-build libusb-1.0-0-dev # Linux
```

For Windows, it's a little more complex. While there's more than one way, we've verified the following procedure to work.

- Use [winget](https://docs.microsoft.com/en-us/windows/package-manager/winget/) to install [git](https://winget.run/pkg/Git/Git).
- Use [vcpkg](https://github.com/libusb/libusb/wiki/Windows#vcpkg-port) to install `libusb` with the right triplet for your setup (ex. `x64-windows`).
    - Set `CMAKE_TOOLCHAIN_FILE` environment variable to the location of `vcpkg\scripts\buildsystems\vcpkg.cmake`.
- Use [Zadig](https://zadig.akeo.ie) to install the [WinUSB](https://github.com/libusb/libusb/wiki/Windows#driver-installation) driver on any `JABI USB` devices.
- Install [Visual Studio C++](https://visualstudio.microsoft.com/vs/features/cplusplus/) as a C++ compiler. It's also an IDE.
- Install [CMake](https://cmake.org/download/) and make sure it's added to `PATH`.

#### libjabi

`libjabi` is provided as a CMake library. A barebones example is provided in `tests/libjabi` and can be run as follows.

```
cd tests/libjabi
mkdir build && cd build
cmake ..
cmake --build .
./main
```

#### pyjabi

`pyjabi` can be installed from a local directory using `pip` for easy management. An example application is in `tests/pyjabi/main.py`.

```
pip install clients/pyjabi
python tests/pyjabi/main.py
```

## TODO

The following clients.

- gRPC client/server bridging C++ to a network
    - iOS app in React Native or Swift
    - Android app in React Native or Kotlin (USB too?)
    - QT6 cross-platform app
    - Website using Svelte JS (WebUSB too?)

The following peripherals.

- CAN FD
    - allow one filter bc USB FS is slow
- LIN
- SPI master
- I2C master
- UART
- GPIO
- ADC
- DAC

Fun things to look into one day.

- CAN DBC
- DFU options beyond USB for MCUboot
- Alternative functions for pins
- Improve security and robustness (encryption? CRC?)
- gRPC server on the microcontroller (in the roadmap for Zephyr it seems)
- multiple USB interfaces for multiple clients (raw USB is \~25% faster than CDC-ACM)
- Ethernet support
    - Listen for TCP on port 42069 and dispatch to a thread pool, refuse if all used
- BLE and WiFi support (need to make a dev board)
- On-device web server
