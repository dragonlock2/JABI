#ifndef JABI_PERIPHERALS_METADATA_H
#define JABI_PERIPHERALS_METADATA_H

#include <jabi/interfaces.h>

typedef uint8_t metadata_serial_resp_t;

PACKED(metadata_num_inst_req_t,
    uint16_t periph_id;
);

PACKED(metadata_num_inst_resp_t,
    uint16_t num_idx;
);

typedef uint8_t metadata_echo_req_t;
typedef uint8_t metadata_echo_resp_t;

PACKED(metadata_req_max_size_resp_t,
    uint16_t size;
);

PACKED(metadata_resp_max_size_resp_t,
    uint16_t size;
);

typedef uint8_t metadata_custom_req_t;
typedef uint8_t metadata_custom_resp_t;

/* Function indices */
#define METADATA_SERIAL_ID        0
#define METADATA_NUM_INST_ID      1
#define METADATA_ECHO_ID          2
#define METADATA_REQ_MAX_SIZE_ID  3
#define METADATA_RESP_MAX_SIZE_ID 4
#define METADATA_CUSTOM_ID        5

#endif // JABI_PERIPHERALS_METADATA_H
