#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <libjabi/device.h>
#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

namespace py = pybind11;
using namespace pybind11::literals;
using namespace jabi;

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
        .def("can_state", &Device::can_state, "idx"_a=0);
        // TODO write
        // TODO read abstract out the num_left to can_read_simple, return None if empty

    /* Interfaces */
    py::class_<USBInterface>(m, "USBInterface")
        .def("list_devices", &USBInterface::list_devices);

    py::class_<UARTInterface>(m, "UARTInterface")
        .def("get_device", &UARTInterface::get_device);

    /* Metadata */
    m.attr("METADATA_ID") = METADATA_ID;
    m.attr("CAN_ID") = CAN_ID;

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
        .def_readwrite("id", &CANMessage::id)
        .def_readwrite("ext", &CANMessage::ext)
        .def_readwrite("fd", &CANMessage::fd)
        .def_readwrite("brs", &CANMessage::brs)
        .def_readwrite("rtr", &CANMessage::rtr)
        .def_readwrite("data", &CANMessage::data) // Note can't set individual elements
        .def("__repr__", [](const CANMessage &m){
            std::stringstream s; s << m; return s.str(); });

    // TODO diff constructors
}
