#ifndef PERIPHERALS_METADATA_H
#define PERIPHERALS_METADATA_H

#include <stdint.h>
#include <interfaces.h>

typedef uint8_t* metadata_get_serial_resp_t;

typedef struct {
    uint16_t periph_id;
} __attribute__((packed)) metadata_get_num_inst_req_t;

typedef struct {
    uint16_t num_idx;
} __attribute__((packed)) metadata_get_num_inst_resp_t;

CHECK_REQ_PAYLOAD_SIZE(metadata_get_num_inst_req_t);
CHECK_RESP_PAYLOAD_SIZE(metadata_get_num_inst_resp_t);

/* Function indices */
#define PERIPH_GET_SERIAL_ID   0
#define PERIPH_GET_NUM_INST_ID 1

#endif // PERIPHERALS_METADATA_H