#ifndef JABI_PERIPHERALS_METADATA_H
#define JABI_PERIPHERALS_METADATA_H

#include <stdint.h>
#include <jabi/interfaces.h>

typedef uint8_t* metadata_get_serial_resp_t;

typedef struct {
    uint16_t periph_id;
} __attribute__((packed)) metadata_get_num_inst_req_t;

typedef struct {
    uint16_t num_idx;
} __attribute__((packed)) metadata_get_num_inst_resp_t;

typedef uint8_t* metadata_echo_req_t;
typedef uint8_t* metadata_echo_resp_t;

/* Function indices */
#define METADATA_GET_SERIAL_ID   0
#define METADATA_GET_NUM_INST_ID 1
#define METADATA_ECHO_ID         2

#endif // JABI_PERIPHERALS_METADATA_H
