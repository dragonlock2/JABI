#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

using namespace std::chrono_literals;

void testDevice(jabi::Device d) {
    /* Metadata */
    std::cout << "SN=" << d.serial();
    std::cout << " num_meta=" << d.num_inst(jabi::METADATA_ID);
    std::cout << " echo=" << d.echo("❤️");
    std::cout << std::endl;

    /* CAN */
    auto lim = d.num_inst(jabi::CAN_ID);
    for (auto i = 0; i < lim; i++) {
        std::cout << "Capturing all messages on CAN" << i << std::endl;
        d.can_set_filter(0, 0, 0, 0, i);
        d.can_set_rate(125000, 1000000, i);
        d.can_set_mode(jabi::CANMode::NORMAL, i);

        jabi::CANState state = d.can_state(i);
        std::cout << "state=" << state.state << " tx_err=" << state.tx_err;
        std::cout << " rx_err" << state.rx_err << std::endl;

        std::cout << "Sending a message" << std::endl;
        jabi::CANMessage msg(0x69, std::vector<uint8_t>{1, 2, 3}, false, false);
        d.can_write(msg, i);

        std::this_thread::sleep_for(100ms); // wait for some messages
        while (d.can_read(msg, i) != -1) {
            std::cout << "Got msg: " << msg << std::endl;
        }
    }
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
