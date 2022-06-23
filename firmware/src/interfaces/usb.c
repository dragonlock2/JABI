#include <sys/byteorder.h>
#include <usb/usb_device.h>
#include <jabi.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(iface_usb, CONFIG_LOG_DEFAULT_LEVEL);

/* Driver will attach to interface descriptor w/ following properties
 *   - bInterfaceClass of 0xFF (Vendor Specific)
 *   - iInterface string descriptor of "JABI_USB"
 *   - only 2 bulk transfer endpoints
 *   - responds to metadata requests on endpoints (within a timeout)
 *
 * Packets are sent raw and split up among multiple bulk transfers
 */

#if DT_PROP(JABI_IFACE_NODE, usb)

#define JABI_IF_STR "JABI_USB"

#define AUTO_EP_IN  0x80
#define AUTO_EP_OUT 0x00

#ifdef CONFIG_USB_DC_HAS_HS_SUPPORT
#define MPS 512
#else
#define MPS 64
#endif // IS_ENABLED(CONFIG_USB_DC_HAS_HS_SUPPORT)

#define UTF16LE_LEN(s) (sizeof(s) * 2 - 2)

extern int usb_get_str_descriptor_idx(void *ptr);

struct usb_jabi_config {
    struct usb_if_descriptor if0;
    struct usb_ep_descriptor if0_out_ep;
    struct usb_ep_descriptor if0_in_ep;
} __packed;

struct usb_jabi_str {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[UTF16LE_LEN(JABI_IF_STR)];
} __packed;

USBD_CLASS_DESCR_DEFINE(primary, 0) struct usb_jabi_config usb_cfg = {
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
        .wMaxPacketSize = sys_cpu_to_le16(MPS),
        .bInterval = 0x00,
    },

    .if0_in_ep = {
        .bLength = sizeof(struct usb_ep_descriptor),
        .bDescriptorType = USB_DESC_ENDPOINT,
        .bEndpointAddress = AUTO_EP_IN,
        .bmAttributes = USB_DC_EP_BULK,
        .wMaxPacketSize = sys_cpu_to_le16(MPS),
        .bInterval = 0x00,
    },
};

USBD_STRING_DESCR_USER_DEFINE(primary) struct usb_jabi_str usb_if_str = {
    .bLength = (UTF16LE_LEN(JABI_IF_STR) + 2),
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

#define JABI_EP_OUT (ep_cfg[0].ep_addr)
#define JABI_EP_IN  (ep_cfg[1].ep_addr)

static void usb_jabi_interface_config(struct usb_desc_header *head,
                                      uint8_t bInterfaceNumber) {
    ARG_UNUSED(head);
    usb_cfg.if0.bInterfaceNumber = bInterfaceNumber;
    usb_cfg.if0.iInterface = usb_get_str_descriptor_idx(&usb_if_str);
}

USBD_DEFINE_CFG_DATA(usb_config) = {
    .usb_device_description = NULL,
    .interface_descriptor = &usb_cfg.if0,
    .interface_config = usb_jabi_interface_config,
    .cb_usb_status = NULL,
    .interface = {
        .class_handler = NULL,
        .vendor_handler = NULL,
        .custom_handler = NULL,
    },
    .num_endpoints = ARRAY_SIZE(ep_cfg),
    .endpoint = ep_cfg,
};

static void usb_init() {}

static void usb_get_req(iface_req_t *req) {
    while (1) { // loop until packet received
        req->payload_len = 0;
        int len = usb_transfer_sync(JABI_EP_OUT, (uint8_t*) req,
                                    sizeof(iface_req_t), USB_TRANS_READ);
        if (len < 0) {
            LOG_ERR("transfer failed");
            continue;
        }
        iface_req_to_le(req);
        if (req->payload_len > REQ_PAYLOAD_MAX_SIZE || 
            len != (IFACE_REQ_HDR_SIZE + req->payload_len)) {
            LOG_ERR("invalid request packet length %d", req->payload_len);
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
    int len = usb_transfer_sync(JABI_EP_IN, (uint8_t*) resp, 
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
