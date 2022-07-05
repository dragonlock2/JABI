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
    std::cout << " num_meta=" << d.num_inst(jabi::InstID::METADATA);
    std::cout << " echo=" << d.echo("❤️");
    std::cout << std::endl;

    /* CAN */
    auto lim = d.num_inst(jabi::InstID::CAN);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tListening only to 0x69420 messages on CAN " << i << std::endl;
        d.can_set_rate(125000, 1000000, i);
        d.can_set_filter(0x69420, 0xFFFFF, 0, 0, i);
        d.can_set_mode(jabi::CANMode::NORMAL, i);
        auto s = d.can_state(i);
        std::cout << "\tstate: " << s.state << " tx_err: " << s.tx_err;
        std::cout << " rx_err: " << s.rx_err << std::endl;

        d.can_write(jabi::CANMessage(0x69420, std::vector<uint8_t>{69, 42}), i);
        d.can_write(jabi::CANMessage(0x69420, 2), i);
        std::cout << "\tSent some messages" << std::endl;

        std::this_thread::sleep_for(500ms);
        std::cout << "\tPrinting received messsages" << std::endl;
        jabi::CANMessage msg;

        // less LOC, but one extra check
        while (d.can_read(msg, i) != -1) {
            std::cout << "\t " << msg << std::endl;
        }
    }
    std::cout << std::endl;

    /* I2C */
    lim = d.num_inst(jabi::InstID::I2C);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tScanning for devices on I2C " << i << std::endl;
        d.i2c_set_freq(jabi::I2CFreq::STANDARD, i);
        for (int j = 0; j < 128; j++) {
            try {
                d.i2c_write(j, std::vector<uint8_t>(), i);
                // d.i2c_read(j, 0, i);
                std::cout << "\t Found " << j << std::endl;
            } catch(const std::runtime_error&) {}
        }
    }
    std::cout << std::endl;

    /* PWM (GPIO overrides it until reset) */
    lim = d.num_inst(jabi::InstID::PWM);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tFlashing PWM " << i << " at 1Hz" << std::endl;
        d.pwm_write(i, 0.5, 1.0);
        std::this_thread::sleep_for(100ms); // add offset
    }
    if (lim > 0) {
        std::this_thread::sleep_for(3s);
    }
    std::cout << std::endl;

    /* GPIO */
    lim = d.num_inst(jabi::InstID::GPIO);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tFlashing GPIO " << i << std::endl;
        d.gpio_set_mode(i, jabi::GPIODir::OUTPUT);
        for (auto j = 0; j < 6; j++) {
            d.gpio_write(i, 0);
            std::this_thread::sleep_for(25ms);
            d.gpio_write(i, 1);
            std::this_thread::sleep_for(25ms);
        }
    }
    for (auto i = 0; i < lim; i++) {
        d.gpio_set_mode(i, jabi::GPIODir::INPUT, jabi::GPIOPull::UP);
        std::cout << "\tRead GPIO " << i << " w/ pullups: " << d.gpio_read(i) << std::endl;
    }
    std::cout << std::endl;

    /* ADC */
    lim = d.num_inst(jabi::InstID::ADC);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tRead ADC " << i << " as " << d.adc_read(i) << "mV" << std::endl;
    }
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
