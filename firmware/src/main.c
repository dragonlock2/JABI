#include <zephyr.h>
#include <usb/usb_device.h>
#include <jabi.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(jabi, CONFIG_LOG_DEFAULT_LEVEL);

#define STACK_SIZE 2048

extern const struct iface_api_t *interfaces[];
K_THREAD_STACK_ARRAY_DEFINE(thread_stack, NUM_INTERFACES, STACK_SIZE);
struct k_thread thread_data[NUM_INTERFACES];

void process_interface(void* p1, void* p2, void* p3) {
    const struct iface_api_t *iface = (const struct iface_api_t*) p1;
    
    LOG_INF("started processing interface %s", iface->name);
    iface->init();

    iface_req_t req;
    iface_resp_t resp;
    while (1) {
        /* CPU endianness assumed for non-payload members */
        iface->get_req(&req);
        LOG_DBG("%s recvd msg id: %d idx: %d len: %d",
                iface->name, req.periph_id, req.periph_idx, req.payload_len);

        // TODO dispatch to correct peripheral
        resp.retcode = 0;
        resp.payload_len = 4;
        for (int i = 0; i < resp.payload_len; i++) {
            resp.payload[i] = i;
        }

        iface->send_resp(&resp);
    }
}

int main() {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    usb_enable(NULL);
#endif

    for (int i = 0; i < NUM_INTERFACES; i++) {
        k_thread_create(&thread_data[i], thread_stack[i], STACK_SIZE,
                        process_interface, (void*) interfaces[i], NULL, NULL,
                        K_PRIO_PREEMPT(0), 0, K_NO_WAIT);
    }
    return 0;
}
