#include <iostream>
#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

void testDevice(jabi::Device d) {
    std::cout << "SN=" << d.get_serial();
    std::cout << " num_meta=" << d.get_num_inst(0);
    std::cout << " echo=" << d.echo("❤️");
    std::cout << std::endl;
}

int main() {
    for (auto d : jabi::USBInterface::list_devices()) {
        std::cout << "Found USB: ";
        testDevice(d);
    }

    std::cout << "Found UART: ";
    testDevice(jabi::UARTInterface::get_device("COM5", 230400));
    std::cout << "done!" << std::endl;

    return 0;
}
