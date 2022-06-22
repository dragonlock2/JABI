#include <jabi.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(iface_usb, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_PROP(JABI_IFACE_NODE, usb)

static void usb_init() {
    LOG_INF("Called init");
}

static void usb_get_req(iface_req_t *req) {
    LOG_INF("got request");
}

static void usb_send_resp(iface_resp_t *resp) {
    LOG_INF("sent response");
    while (1) { k_msleep(500); };
}

const struct iface_api_t usb_iface_api = {
    .init = usb_init,
    .get_req = usb_get_req,
    .send_resp = usb_send_resp,
    .name = "USB"
};

#endif // DT_PROP(JABI_IFACE_NODE, usb)
