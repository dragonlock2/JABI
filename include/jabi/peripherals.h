#ifndef JABI_PERIPHERALS_H
#define JABI_PERIPHERALS_H

#include <stdint.h>

typedef int16_t (*periph_func_t)(uint16_t idx, uint8_t *req, uint16_t req_len,
                                 uint8_t *resp, uint16_t *resp_len);

struct periph_api_t {
    periph_func_t *fns;
    uint16_t num_fns;
    uint16_t num_idx;
    const char *name;
};

/* Peripheral indices */
#define PERIPH_METADATA_ID 0

#define NUM_PERIPHERALS 1

/* Helpers */
#define PERIPH_FUNC_DEF(fn) static int16_t fn(uint16_t idx,                      \
                                              uint8_t *req, uint16_t req_len,    \
                                              uint8_t *resp, uint16_t *resp_len)

#define PERIPH_FUNC_GET_ARGS(p, fn) p##_##fn##_req_t *args = (p##_##fn##_req_t*) req
#define PERIPH_FUNC_GET_RET(p, fn)  p##_##fn##_resp_t *ret = (p##_##fn##_resp_t*) resp

#define PERIPH_FUNC_CHECK_ARGS_LEN(p, fn)      \
    if (req_len != sizeof(p##_##fn##_req_t)) { \
        return JABI_INVALID_ARGS_FORMAT_ERR;   \
    }

#define PERIPH_FUNC_CHECK_ARGS_EMPTY \
    if (req_len != 0) { return JABI_INVALID_ARGS_FORMAT_ERR; }

#endif // JABI_PERIPHERALS_H