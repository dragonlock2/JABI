#include <string>
#include "usb.h"

#define USB_TIMEOUT_MS 1000

namespace jabi {

USBInterface::USBInterface(libusb_device_handle *dev, int ifnum, int wMaxPacketSize,
    unsigned char ep_out, unsigned char ep_in)
:
    dev(dev), ifnum(ifnum), wMaxPacketSize(wMaxPacketSize), ep_out(ep_out), ep_in(ep_in)
{}

USBInterface::~USBInterface() {
    libusb_release_interface(dev, ifnum);
    libusb_close(dev);
}

iface_resp_t USBInterface::send_request(iface_req_t req) {
    std::scoped_lock lk(req_lock);

    if (req.payload_len > REQ_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("request payload size too large");
    }

    iface_req_htole(req);

    int sent_len;
    int len = IFACE_REQ_HDR_SIZE + req.payload_len;
    if (libusb_bulk_transfer(dev, ep_out, reinterpret_cast<unsigned char*>(&req),
            len, &sent_len, USB_TIMEOUT_MS) < 0) {
        throw std::runtime_error("USB transfer request failed");
    }
    if (sent_len != len) {
        throw std::runtime_error("wrong USB transfer request length");
    }
    if (len % wMaxPacketSize == 0) { // manually send ZLP
        if (libusb_bulk_transfer(dev, ep_out, NULL, 0, NULL, USB_TIMEOUT_MS) < 0) {
            throw std::runtime_error("USB transfer ZLP request failed");
        }
    }

    int recv_len;
    iface_resp_t resp;
    resp.payload_len = 0;
    if (libusb_bulk_transfer(dev, ep_in, reinterpret_cast<unsigned char*>(&resp),
            sizeof(iface_resp_t), &recv_len, USB_TIMEOUT_MS) < 0) {
        throw std::runtime_error("USB transfer response failed");
    }

    iface_resp_letoh(resp);

    if (recv_len != (int) IFACE_RESP_HDR_SIZE + resp.payload_len) {
        throw std::runtime_error("wrong USB transfer response length");
    }
    if (resp.retcode != 0 || resp.payload_len > RESP_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("bad response " + std::to_string(resp.retcode));
    }

    return resp;
}

/* Driver will attach to an interface descriptor w/ following properties
 *   - bAlternateSetting of 0 (default)
 *   - bInterfaceClass of 0xFF (Vendor Specific)
 *   - iInterface string descriptor of "JABI USB"
 *   - only 2 bulk transfer endpoints (1 IN, 1 OUT)
 *   - responds to a get_serial() request
 */
std::vector<Device> USBInterface::list_devices() {
    if (libusb_init(NULL) < 0) {
        throw std::runtime_error("libusb failed init");
    }

    libusb_device **devs;
    ssize_t num = libusb_get_device_list(NULL, &devs);
    if (num < 0) {
        libusb_exit(NULL);
        throw std::runtime_error("libusb couldn't get device list");
    }

    std::vector<Device> jabis;
    for (auto i = 0; i < num; i++) {
        struct libusb_config_descriptor *cfg;
        if (libusb_get_active_config_descriptor(devs[i], &cfg) < 0) {
            continue;
        }

        for (auto j = 0; j < cfg->bNumInterfaces; j++) {
            struct libusb_interface if_descs = cfg->interface[j];
            if (if_descs.num_altsetting == 0) {
                continue;
            }

            struct libusb_interface_descriptor if_desc = if_descs.altsetting[0];
            if (if_desc.bInterfaceClass != LIBUSB_CLASS_VENDOR_SPEC ||
                if_desc.iInterface == 0 || if_desc.bNumEndpoints != 2 ||
                if_desc.bAlternateSetting != 0 ) {
                continue;
            }

            struct libusb_endpoint_descriptor ep0 = if_desc.endpoint[0];
            struct libusb_endpoint_descriptor ep1 = if_desc.endpoint[1];
            if ((ep0.bmAttributes & 0x03) != LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK ||
                (ep1.bmAttributes & 0x03) != LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK ||
                !((ep0.bEndpointAddress ^ ep1.bEndpointAddress) & 0x80)) {
                continue;
            }

            libusb_device_handle *dev;
            if(libusb_open(devs[i], &dev) < 0) {
                break;
            }

            unsigned char str[256];
            if (libusb_get_string_descriptor_ascii(dev, if_desc.iInterface, str, 256) < 0) {
                libusb_close(dev);
                break;
            }
            if (std::string(reinterpret_cast<char*>(str)) != "JABI USB") {
                libusb_close(dev);
                continue;
            }

            if (libusb_claim_interface(dev, if_desc.bInterfaceNumber) < 0) {
                libusb_close(dev);
                continue;
            }

            auto ep_out = ep0, ep_in = ep1;
            if (ep0.bEndpointAddress & 0x80) { std::swap(ep_out, ep_in); }

            Device jabi = Interface::makeDevice(
                std::shared_ptr<USBInterface>(
                    new USBInterface(
                        dev,
                        if_desc.bInterfaceNumber,
                        ep_out.wMaxPacketSize,
                        ep_out.bEndpointAddress,
                        ep_in.bEndpointAddress
                    )
                )
            );
            try {
                jabi.serial();
            } catch(const std::runtime_error&) {
                // reset device and try one more time
                if (libusb_reset_device(dev) < 0) {
                    break;
                }
                try {
                    jabi.serial();
                } catch(const std::runtime_error&) {
                    break;
                }
            }
            jabis.push_back(jabi);
        }

        libusb_free_config_descriptor(cfg);
    }

    libusb_free_device_list(devs, 1);
    return jabis;
}

};
