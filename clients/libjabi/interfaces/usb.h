#ifndef LIBJABI_INTERFACES_USB_H
#define LIBJABI_INTERFACES_USB_H

#include <libusb-1.0/libusb.h>
#include "interface.h"

namespace jabi {

class USBInterface : public Interface {
public:
    ~USBInterface();

    iface_resp_t send_request(iface_req_t req);
    static std::vector<Device> list_devices();

private:
    USBInterface(libusb_device_handle *dev, int ifnum, int wMaxPacketSize,
        unsigned char ep_out, unsigned char ep_in);
    
    libusb_device_handle *dev;
    int ifnum;
    int wMaxPacketSize; // for OUT transfers
    unsigned char ep_out;
    unsigned char ep_in;
};

};

#endif // LIBJABI_INTERFACES_USB_H
