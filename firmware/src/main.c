#include <zephyr.h>
#include <usb/usb_device.h>
#include <stdlib.h>
#include <jabi.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(jabi, CONFIG_LOG_DEFAULT_LEVEL);

#define STACK_SIZE 2048

extern const struct iface_api_t *interfaces[];
extern const struct periph_api_t *peripherals[];

K_THREAD_STACK_ARRAY_DEFINE(thread_stack, NUM_INTERFACES, STACK_SIZE);
struct k_thread thread_data[NUM_INTERFACES];

struct k_sem *peripheral_locks[NUM_PERIPHERALS];

void process_interface(void* p1, void* p2, void* p3) {
    const struct iface_api_t *iface = (const struct iface_api_t*) p1;
    
    if (iface->init()) {
        LOG_ERR("failed to start interface %s", iface->name);
        return;
    }
    LOG_INF("started processing interface %s", iface->name);

    iface_req_t req;
    iface_resp_t resp;
    while (1) {
        /* CPU endianness assumed for non-payload members */
        iface->get_req(&req);
        LOG_DBG("%s recvd msg id: %d idx: %d fn: %d",
                iface->name, req.periph_id, req.periph_idx, req.periph_fn);

        uint16_t payload_len = 0;

        if (req.periph_id >= NUM_PERIPHERALS) {
            LOG_ERR("%s invalid peripheral id %d", iface->name, req.periph_id);
            resp.retcode = JABI_NOT_SUPPORTED_ERR;
            goto send_resp;
        }

        const struct periph_api_t *api = peripherals[req.periph_id];

        if (req.periph_idx >= api->num_idx) {
            LOG_ERR("%s invalid peripheral index %d", iface->name, req.periph_idx);
            resp.retcode = JABI_NOT_SUPPORTED_ERR;
            goto send_resp;
        }

        if (req.periph_fn >= api->num_fns) {
            LOG_ERR("%s invalid peripheral function id %d", iface->name, req.periph_fn);
            resp.retcode = JABI_NOT_SUPPORTED_ERR;
            goto send_resp;
        }

        k_sem_take(&peripheral_locks[req.periph_id][req.periph_idx], K_FOREVER);
        
        resp.retcode = api->fns[req.periph_fn](req.periph_idx, 
                                               req.payload, req.payload_len,
                                               resp.payload, &payload_len);

        k_sem_give(&peripheral_locks[req.periph_id][req.periph_idx]);

        if (resp.retcode) {
            LOG_ERR("%s peripheral function error %d", iface->name, resp.retcode);
            payload_len = 0;
        }

send_resp:
        resp.payload_len = payload_len;
        iface->send_resp(&resp);
    }
}

int main() {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    usb_enable(NULL);
#endif

    for (int i = 0; i < NUM_PERIPHERALS; i++) {
        if (peripherals[i]->num_idx == 0) {
            continue;
        }
        struct k_sem *locks = malloc(sizeof(struct k_sem) * peripherals[i]->num_idx);
        if (locks == NULL) {
            LOG_ERR("failed to allocate locks, time to die");
            return -1;
        }
        for (int j = 0; j < peripherals[i]->num_idx; j++) {
            k_sem_init(&locks[j], 1, 1);
        }
        peripheral_locks[i] = locks;
    }

    for (int i = 0; i < NUM_PERIPHERALS; i++) {
        for (int j = 0; j < peripherals[i]->num_idx; j++) {
            if (peripherals[i]->init(j)) {
                LOG_ERR("failed to initialize peripheral %s%d, time to die",
                    peripherals[i]->name, j);
                return -1;
            }
        }
    }

    for (int i = 0; i < NUM_INTERFACES; i++) {
        k_thread_create(&thread_data[i], thread_stack[i], STACK_SIZE,
                        process_interface, (void*) interfaces[i], NULL, NULL,
                        K_PRIO_PREEMPT(0), 0, K_NO_WAIT);
    }
    return 0;
}
