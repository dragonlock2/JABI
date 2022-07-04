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
        .def("can_read", &can_read_simple, "idx"_a=0);

    /* Interfaces */
    py::class_<USBInterface>(m, "USBInterface")
        .def("list_devices", &USBInterface::list_devices);

    py::class_<UARTInterface>(m, "UARTInterface")
        .def("get_device", &UARTInterface::get_device);

    /* Metadata */
    py::enum_<InstID>(m, "InstID")
        .value("METADATA", InstID::METADATA)
        .value("CAN", InstID::CAN);

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
}
