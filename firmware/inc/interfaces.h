#ifndef INTERFACES_H
#define INTERFACES_H

#include <stdint.h>

#define REQ_PAYLOAD_MAX_SIZE   512
#define RESP_PAYLOAD_MAX_SIZE  512

typedef struct {
    uint16_t periph_id;
    uint16_t periph_idx;
    uint16_t periph_fn;
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

#define IFACE_REQ_HDR_SIZE  (sizeof(iface_req_t) - REQ_PAYLOAD_MAX_SIZE)
#define IFACE_RESP_HDR_SIZE (sizeof(iface_resp_t) - RESP_PAYLOAD_MAX_SIZE)

extern void iface_req_to_le(iface_req_t *req);
extern void iface_resp_to_le(iface_resp_t *resp);

#define CHECK_REQ_PAYLOAD_SIZE(x)  BUILD_ASSERT(sizeof(x) < REQ_PAYLOAD_MAX_SIZE)
#define CHECK_RESP_PAYLOAD_SIZE(x) BUILD_ASSERT(sizeof(x) < RESP_PAYLOAD_MAX_SIZE)

enum {
    JABI_NO_ERR = 0,
    JABI_NOT_SUPPORTED_ERR,
    JABI_INVALID_ARGS_FORMAT_ERR,
};

#endif // INTERFACES_H
