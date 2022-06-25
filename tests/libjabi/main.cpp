#include <iostream>
#include <libjabi/interfaces/usb.h>

int main() {
    for (auto d : jabi::USBInterface::list_devices()) {
        std::cout << "Found SN=" << d.get_serial();
        std::cout << " num_meta=" << d.get_num_inst(0);
        std::cout << " echo=" << d.echo("❤️");
        std::cout << std::endl;
    }

    return 0;
}
