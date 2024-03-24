#ifndef LIBJABI_INTERFACES_USB_H
#define LIBJABI_INTERFACES_USB_H

#include "interface.h"

namespace jabi {

class USBInterface : public Interface {
public:
    ~USBInterface();

    iface_dynamic_resp_t send_request(iface_dynamic_req_t req);
    static std::vector<Device> list_devices();

private:
    USBInterface(void *dev, int ifnum, int wMaxPacketSize,
        unsigned char ep_out, unsigned char ep_in);
    
    void *dev; // libusb_device_handle* but libusb.h and pyconfig.h conflict :(
    int ifnum;
    int wMaxPacketSize; // for OUT transfers
    unsigned char ep_out;
    unsigned char ep_in;
};

};

#endif // LIBJABI_INTERFACES_USB_H
