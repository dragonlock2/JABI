#ifndef INTERFACE_COMMON_H
#define INTERFACE_COMMON_H

#include <stdint.h>

#define REQ_PAYLOAD_MAX_SIZE   512
#define RESP_PAYLOAD_MAX_SIZE  512

typedef struct {
    uint8_t periph_id;
    uint8_t periph_idx;
    uint16_t payload_len;
    uint8_t payload[REQ_PAYLOAD_MAX_SIZE];
} __attribute__((packed)) iface_req_t;

typedef struct {
    int16_t retcode;
    uint16_t payload_len;
    uint8_t payload[RESP_PAYLOAD_MAX_SIZE];
} __attribute__((packed)) iface_resp_t;

typedef void (*iface_init_t)(void);
typedef void (*iface_get_req_t)(iface_req_t *req);
typedef void (*iface_send_resp_t)(iface_resp_t *resp);

struct iface_api_t {
    iface_init_t init;
    iface_get_req_t get_req;
    iface_send_resp_t send_resp;
    const char *name;
};

#endif // INTERFACE_COMMON_H
