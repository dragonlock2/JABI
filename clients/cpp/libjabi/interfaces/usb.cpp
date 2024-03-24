#include <cstring>
#include <string>
#include <libusb.h>
#include "usb.h"

#define USB_TIMEOUT_MS 3000

namespace jabi {

USBInterface::USBInterface(void *dev, int ifnum, int wMaxPacketSize,
    unsigned char ep_out, unsigned char ep_in)
:
    dev(dev), ifnum(ifnum), wMaxPacketSize(wMaxPacketSize), ep_out(ep_out), ep_in(ep_in)
{}

USBInterface::~USBInterface() {
    libusb_release_interface(static_cast<libusb_device_handle*>(dev), ifnum);
    libusb_close(static_cast<libusb_device_handle*>(dev));
}

iface_dynamic_resp_t USBInterface::send_request(iface_dynamic_req_t req) {
    std::scoped_lock lk(req_lock);

    if (req.msg.payload_len > req_max_size ||
        req.msg.payload_len != req.payload.size()) {
        throw std::runtime_error("request payload size bad");
    }

    iface_req_htole(req.msg);

    // transfer must be contiguous, allocate buffer from heap (MSVC complains about stack)
    auto req_buffer = std::make_unique<uint8_t[]>(IFACE_REQ_HDR_SIZE + req_max_size);
    iface_req_t* req_msg = reinterpret_cast<iface_req_t*>(req_buffer.get());
    memcpy(req_msg, &req.msg, IFACE_REQ_HDR_SIZE);
    memcpy(req_msg->payload, req.payload.data(), req.payload.size());

    int sent_len;
    int len = static_cast<int>(IFACE_REQ_HDR_SIZE + req.payload.size());
    if (libusb_bulk_transfer(static_cast<libusb_device_handle*>(dev), ep_out, reinterpret_cast<unsigned char*>(req_msg),
            len, &sent_len, USB_TIMEOUT_MS) < 0) {
        throw std::runtime_error("USB transfer request failed");
    }
    if (sent_len != len) {
        throw std::runtime_error("wrong USB transfer request length");
    }
    if (len % wMaxPacketSize == 0) { // manually send ZLP
        if (libusb_bulk_transfer(static_cast<libusb_device_handle*>(dev), ep_out, NULL, 0, NULL, USB_TIMEOUT_MS) < 0) {
            throw std::runtime_error("USB transfer ZLP request failed");
        }
    }

    // transfer must be contiguous, allocate buffer from heap (MSVC complains about stack)
    auto resp_buffer = std::make_unique<uint8_t[]>(IFACE_RESP_HDR_SIZE + resp_max_size);
    iface_resp_t* resp_msg = reinterpret_cast<iface_resp_t*>(resp_buffer.get());

    int recv_len;
    resp_msg->payload_len = 0;
    if (libusb_bulk_transfer(static_cast<libusb_device_handle*>(dev), ep_in, reinterpret_cast<unsigned char*>(resp_msg),
            static_cast<int>(IFACE_RESP_HDR_SIZE + resp_max_size), &recv_len, USB_TIMEOUT_MS) < 0) {
        throw std::runtime_error("USB transfer response failed");
    }

    iface_resp_letoh(*resp_msg);

    if (recv_len != static_cast<int>(IFACE_RESP_HDR_SIZE + resp_msg->payload_len)) {
        throw std::runtime_error("wrong USB transfer response length");
    }
    if (resp_msg->retcode != 0 || resp_msg->payload_len > resp_max_size) {
        throw std::runtime_error("bad response " + std::to_string(resp_msg->retcode));
    }

    iface_dynamic_resp_t resp;
    memcpy(&resp.msg, resp_msg, IFACE_RESP_HDR_SIZE);
    resp.payload = std::vector<uint8_t>(resp.msg.payload_len, 0);
    memcpy(resp.payload.data(), &resp_msg->payload, resp.payload.size());

    return resp;
}

/* Driver will attach to an interface descriptor w/ following properties
 *   - bAlternateSetting of 0 (default)
 *   - bInterfaceClass of 0xFF (Vendor Specific)
 *   - iInterface string descriptor of "JABI USB"
 *   - only 2 bulk transfer endpoints (1 IN, 1 OUT)
 *   - responds to a req_max_size() and resp_max_size() request
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
            if (libusb_open(devs[i], &dev) < 0) {
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

            std::shared_ptr<USBInterface> iface(
                new USBInterface(
                    dev,
                    if_desc.bInterfaceNumber,
                    ep_out.wMaxPacketSize,
                    ep_out.bEndpointAddress,
                    ep_in.bEndpointAddress
                )
            );
            Device jabi = Interface::make_device(iface);
            try {
                if ((iface->req_max_size = jabi.req_max_size()) < REQ_PAYLOAD_MAX_SIZE ||
                    (iface->resp_max_size = jabi.resp_max_size()) < RESP_PAYLOAD_MAX_SIZE) {
                    throw std::runtime_error("maximum packet size too small");
                }
            } catch(const std::runtime_error&) {
                // reset device and try one more time
                if (libusb_reset_device(dev) < 0) {
                    break;
                }
                try {
                    if ((iface->req_max_size = jabi.req_max_size()) < REQ_PAYLOAD_MAX_SIZE ||
                        (iface->resp_max_size = jabi.resp_max_size()) < RESP_PAYLOAD_MAX_SIZE) {
                        throw std::runtime_error("maximum packet size too small");
                    }
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
