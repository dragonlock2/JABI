#ifndef JABI_INTERFACES_H
#define JABI_INTERFACES_H

#include <stdint.h>

#ifdef _MSC_VER
#define PACKED(c, m) __pragma(pack(push,1)) typedef struct {m} c __pragma(pack(pop))
#else
#define PACKED(c, m) typedef struct {m} __attribute__((packed)) c
#endif // _MSC_VER

#define REQ_PAYLOAD_MAX_SIZE   512
#define RESP_PAYLOAD_MAX_SIZE  512

PACKED(iface_req_t,
    uint16_t periph_id;
    uint16_t periph_idx;
    uint16_t periph_fn;
    uint16_t payload_len;
    uint8_t payload[REQ_PAYLOAD_MAX_SIZE];
);

PACKED(iface_resp_t,
    int16_t retcode;
    uint16_t payload_len;
    uint8_t payload[RESP_PAYLOAD_MAX_SIZE];
);

typedef int  (*iface_init_t)(void);
typedef void (*iface_get_req_t)(iface_req_t *req);
typedef void (*iface_send_resp_t)(iface_resp_t *resp);

struct iface_api_t {
    const iface_init_t init;
    const iface_get_req_t get_req;
    const iface_send_resp_t send_resp;
    const char *name;
};

#define IFACE_REQ_HDR_SIZE  (sizeof(iface_req_t) - REQ_PAYLOAD_MAX_SIZE)
#define IFACE_RESP_HDR_SIZE (sizeof(iface_resp_t) - RESP_PAYLOAD_MAX_SIZE)

extern void iface_req_to_le(iface_req_t *req);
extern void iface_resp_to_le(iface_resp_t *resp);

#endif // JABI_INTERFACES_H
