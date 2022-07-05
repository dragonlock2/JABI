#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <libjabi/device.h>
#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

namespace py = pybind11;
using namespace pybind11::literals;
using namespace jabi;

py::object can_read_simple(Device &d, int idx) {
    CANMessage msg;
    if (d.can_read(msg, idx) == -1) {
        return py::none();
    }
    return py::cast(msg);
}

PYBIND11_MODULE(jabi, m) {
    /* Metadata */
    py::enum_<InstID>(m, "InstID")
        .value("METADATA", InstID::METADATA)
        .value("CAN", InstID::CAN)
        .value("I2C", InstID::I2C)
        .value("GPIO", InstID::GPIO)
        .value("PWM", InstID::PWM)
        .value("ADC", InstID::ADC);

    /* CAN */
    py::enum_<CANMode>(m, "CANMode")
        .value("NORMAL", CANMode::NORMAL)
        .value("LOOPBACK", CANMode::LOOPBACK)
        .value("LISTENONLY", CANMode::LISTENONLY);

    py::class_<CANState>(m, "CANState")
        .def_readwrite("state", &CANState::state)
        .def_readwrite("tx_err", &CANState::tx_err)
        .def_readwrite("rx_err", &CANState::rx_err);

    py::class_<CANMessage>(m, "CANMessage")
        .def(py::init<>())
        .def(py::init<int, int, bool, bool>(),
            "id"_a, "req_len"_a, "fd"_a=false, "brs"_a=false)
        .def(py::init<int, std::vector<uint8_t>, bool, bool>(),
            "id"_a, "data"_a, "fd"_a=false, "brs"_a=false)
        .def_readwrite("id", &CANMessage::id)
        .def_readwrite("ext", &CANMessage::ext)
        .def_readwrite("fd", &CANMessage::fd)
        .def_readwrite("brs", &CANMessage::brs)
        .def_readwrite("rtr", &CANMessage::rtr)
        .def_readwrite("data", &CANMessage::data) // Note can't set individual elements
        .def("__repr__", [](const CANMessage &m){
            std::stringstream s; s << m; return s.str(); });

    /* I2C */
    py::enum_<I2CFreq>(m, "I2CFreq")
        .value("STANDARD", I2CFreq::STANDARD)
        .value("FAST", I2CFreq::FAST)
        .value("FAST_PLUS", I2CFreq::FAST_PLUS)
        .value("HIGH", I2CFreq::HIGH)
        .value("ULTRA", I2CFreq::ULTRA);

    /* GPIO */
    py::enum_<GPIODir>(m, "GPIODir")
        .value("INPUT", GPIODir::INPUT)
        .value("OUTPUT", GPIODir::OUTPUT)
        .value("OPEN_DRAIN", GPIODir::OPEN_DRAIN)
        .value("OPEN_SOURCE", GPIODir::OPEN_SOURCE);

    py::enum_<GPIOPull>(m, "GPIOPull")
        .value("NONE", GPIOPull::NONE)
        .value("UP", GPIOPull::UP)
        .value("DOWN", GPIOPull::DOWN)
        .value("BOTH", GPIOPull::BOTH);

    /* Device */
    py::class_<Device>(m, "Device")
        /* Metadata */
        .def("serial", &Device::serial)
        .def("num_inst", &Device::num_inst)
        .def("echo", &Device::echo)

        /* CAN */
        .def("can_set_filter", &Device::can_set_filter,
            "id"_a, "id_mask"_a, "rtr"_a, "rtr_mask"_a, "idx"_a=0)
        .def("can_set_rate", &Device::can_set_rate,
            "bitrate"_a, "bitrate_data"_a, "idx"_a=0)
        .def("can_set_mode", &Device::can_set_mode, "mode"_a, "idx"_a=0)
        .def("can_state", &Device::can_state, "idx"_a=0)
        .def("can_write", &Device::can_write, "msg"_a, "idx"_a=0)
        .def("can_read", &can_read_simple, "idx"_a=0)

        /* I2C */
        .def("i2c_set_freq", &Device::i2c_set_freq, "preset"_a, "idx"_a=0)
        .def("i2c_write", &Device::i2c_write, "addr"_a, "data"_a, "idx"_a=0)
        .def("i2c_read", &Device::i2c_read, "addr"_a, "len"_a, "idx"_a=0)

        /* GPIO */
        .def("gpio_set_mode", &Device::gpio_set_mode, 
            "idx"_a, "dir"_a=GPIODir::INPUT, "pull"_a=GPIOPull::NONE, "init_val"_a=false)
        .def("gpio_write", &Device::gpio_write, "idx"_a, "val"_a)
        .def("gpio_read", &Device::gpio_read, "idx"_a)

        /* PWM */
        .def("pwm_write", &Device::pwm_write)

        /* ADC */
        .def("adc_read", &Device::adc_read);

    /* Interfaces */
    py::class_<USBInterface>(m, "USBInterface")
        .def("list_devices", &USBInterface::list_devices);

    py::class_<UARTInterface>(m, "UARTInterface")
        .def("get_device", &UARTInterface::get_device);
}
