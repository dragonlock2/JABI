#include <zephyr/sys/byteorder.h>
#include <zephyr/usb/usb_device.h>
#include <usb_descriptor.h>
#include <jabi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(iface_usb, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_PROP(JABI_IFACE_NODE, usb)

/* USB configuration */
#define JABI_IF_STR "JABI USB" // host will search for this
#define JABI_VENDOR_CODE (0x69)

USBD_CLASS_DESCR_DEFINE(primary, 0) struct {
    struct usb_if_descriptor if0;
    struct usb_ep_descriptor if0_out_ep;
    struct usb_ep_descriptor if0_in_ep;
} __packed usb_if_desc = {
    .if0 = {
        .bLength = sizeof(struct usb_if_descriptor),
        .bDescriptorType = USB_DESC_INTERFACE,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_BCC_VENDOR,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0,
    },
    .if0_out_ep = {
        .bLength = sizeof(struct usb_ep_descriptor),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = AUTO_EP_OUT,
        .bmAttributes = USB_DC_EP_BULK,
        .wMaxPacketSize = sys_cpu_to_le16(CONFIG_JABI_USB_MPS),
        .bInterval = 0x00,
    },
    .if0_in_ep = {
        .bLength = sizeof(struct usb_ep_descriptor),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = AUTO_EP_IN,
        .bmAttributes = USB_DC_EP_BULK,
        .wMaxPacketSize = sys_cpu_to_le16(CONFIG_JABI_USB_MPS),
        .bInterval = 0x00,
    },
};

USBD_STRING_DESCR_USER_DEFINE(primary) struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[USB_BSTRING_LENGTH(JABI_IF_STR)];
} __packed usb_str_desc = {
    .bLength = USB_STRING_DESCRIPTOR_LENGTH(JABI_IF_STR),
    .bDescriptorType = USB_DESC_STRING,
    .bString = JABI_IF_STR,
};

static struct usb_ep_cfg_data ep_cfg[] = {
    {
        .ep_cb = usb_transfer_ep_callback,
        .ep_addr = AUTO_EP_OUT,
    },
    {
        .ep_cb = usb_transfer_ep_callback,
        .ep_addr = AUTO_EP_IN,
    },
};

static void usb_jabi_interface_config(struct usb_desc_header *head,
                                      uint8_t bInterfaceNumber) {
    ARG_UNUSED(head);
    usb_if_desc.if0.bInterfaceNumber = bInterfaceNumber;
    usb_if_desc.if0.iInterface = usb_get_str_descriptor_idx(&usb_str_desc);
}

static int custom_handler(struct usb_setup_packet *setup, int32_t *len, uint8_t **data) {
    // from https://github.com/pbatard/libwdi/wiki/WCID-Devices
    static uint8_t msos1_str_desc[] = {
        0x12,                   // 18 byte length
        0x03,                   // string type
        0x4D, 0x00, 0x53, 0x00, // "MSFT100" in UTF16LE
        0x46, 0x00, 0x54, 0x00,
        0x31, 0x00, 0x30, 0x00,
        0x30, 0x00,
        JABI_VENDOR_CODE,       // vendor code
        0x00,                   // pad
    };

    // from zephyr/samples/subsys/usb/webusb/src/main.c
    if (usb_reqtype_is_to_device(setup)) {
        return -ENOTSUP;
    }
    if (USB_GET_DESCRIPTOR_TYPE(setup->wValue) == USB_DESC_STRING &&
        USB_GET_DESCRIPTOR_INDEX(setup->wValue) == 0xEE) {
        *data = msos1_str_desc;
        *len = sizeof(msos1_str_desc);
        return 0;
    }
    return -EINVAL;
}

static int vendor_handler(struct usb_setup_packet *setup, int32_t *len, uint8_t **data) {
    // from https://github.com/pbatard/libwdi/wiki/WCID-Devices
    static uint8_t msos1_compatid_desc[] = {
        0x28, 0x00, 0x00, 0x00,                         // 40 byte length
        0x00, 0x01,                                     // version 1.0
        0x04, 0x00,                                     // wIndex
        0x01,                                           // 1 section
        0x00, 0x00, 0x00, 0x00,                         // reserved
        0x00, 0x00, 0x00,
        0x00,                                           // interface this applies to
        0x01,                                           // reserved
        'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,       // compatible ID
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible ID
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // reserved
    };
    msos1_compatid_desc[16] = usb_if_desc.if0.bInterfaceNumber;

    // from zephyr/samples/subsys/usb/webusb/src/main.c
    if (usb_reqtype_is_to_device(setup)) {
        return -ENOTSUP;
    }
    if (setup->bRequest == JABI_VENDOR_CODE && setup->wIndex == 0x0004) {
        *data = msos1_compatid_desc;
		*len = sizeof(msos1_compatid_desc);
		return 0;
    }
    return -EINVAL;
}

USBD_DEFINE_CFG_DATA(usb_config) = {
    .usb_device_description = NULL,
    .interface_descriptor = &usb_if_desc.if0,
    .interface_config = usb_jabi_interface_config,
    .cb_usb_status = NULL,
    .interface = {
        .class_handler = NULL,
        .vendor_handler = vendor_handler,
        .custom_handler = custom_handler,
    },
    .num_endpoints = ARRAY_SIZE(ep_cfg),
    .endpoint = ep_cfg,
};

/* JABI API implementation */
static int usb_init() {
    return 0;
}

static void usb_get_req(iface_req_t *req) {
    while (1) { // loop until packet received
        req->payload_len = 0;
        int len = usb_transfer_sync(ep_cfg[0].ep_addr, (uint8_t*) req,
                                    sizeof(iface_req_t), USB_TRANS_READ);
        if (len < 0) {
            LOG_ERR("transfer failed");
            continue;
        }
        iface_req_to_le(req);
        if (req->payload_len > REQ_PAYLOAD_MAX_SIZE || 
            len != (IFACE_REQ_HDR_SIZE + req->payload_len)) {
            LOG_ERR("invalid request packet length %d %d", req->payload_len, len);
            continue;
        }
        break;
    }
}

static void usb_send_resp(iface_resp_t *resp) {
    if (resp->payload_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("bad resp payload length %d", resp->payload_len);
    }
    iface_resp_to_le(resp);
    size_t expect_len = IFACE_RESP_HDR_SIZE + resp->payload_len;
    int len = usb_transfer_sync(ep_cfg[1].ep_addr, (uint8_t*) resp,
                                expect_len, USB_TRANS_WRITE);
    if (len < 0 || len != expect_len) {
        LOG_ERR("failed to send response %d", len);
    }
}

const struct iface_api_t usb_iface_api = {
    .init = usb_init,
    .get_req = usb_get_req,
    .send_resp = usb_send_resp,
    .name = "USB"
};

#endif // DT_PROP(JABI_IFACE_NODE, usb)
