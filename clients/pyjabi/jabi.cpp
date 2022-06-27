#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <libjabi/device.h>
#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

namespace py = pybind11;
using namespace jabi;

PYBIND11_MODULE(jabi, m) { // name matches CMake module
    py::class_<Device>(m, "Device")
        .def("get_serial", &Device::get_serial)
        .def("get_num_inst", &Device::get_num_inst)
        .def("echo", &Device::echo);

    py::class_<USBInterface>(m, "USBInterface")
        .def("list_devices", &USBInterface::list_devices);

    py::class_<UARTInterface>(m, "UARTInterface")
        .def("get_device", &UARTInterface::get_device);

    m.attr("METADATA_ID") = METADATA_ID;
}
