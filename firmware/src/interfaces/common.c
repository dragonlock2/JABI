#include <device.h>
#include <sys/byteorder.h>
#include <jabi.h>

extern const struct iface_api_t usb_iface_api;

#define GEN_UART_ARRAY(node_id, prop, idx) \
    &uart##idx##_iface_api,
#define GEN_UART_EXTERN(node_id, prop, idx) \
    extern const struct iface_api_t uart##idx##_iface_api;
DT_FOREACH_PROP_ELEM(JABI_IFACE_NODE, uart, GEN_UART_EXTERN)

const struct iface_api_t *interfaces[] = {
#if DT_PROP(JABI_IFACE_NODE, usb)
    &usb_iface_api,
#endif // DT_PROP(JABI_IFACE_NODE, usb)

    DT_FOREACH_PROP_ELEM(JABI_IFACE_NODE, uart, GEN_UART_ARRAY)
};

void iface_req_to_le(iface_req_t *req) {
    req->payload_len = sys_le16_to_cpu(req->payload_len);
}

void iface_resp_to_le(iface_resp_t *resp) {
    resp->retcode = sys_cpu_to_le16(resp->retcode);
    resp->payload_len = sys_cpu_to_le16(resp->payload_len);
}

// note NUM_INTERFACES defined in jabi.h
