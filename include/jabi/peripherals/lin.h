#ifndef JABI_PERIPHERALS_LIN_H
#define JABI_PERIPHERALS_LIN_H

#include <jabi/interfaces.h>

PACKED(lin_set_mode_j_req_t,
    uint8_t mode; /* 0=commander, 1=responder */
);

PACKED(lin_set_rate_req_t,
    uint32_t bitrate;
);

PACKED(lin_set_filter_req_t,
    uint8_t id; /* LIN identifier (0-63) */
    uint8_t checksum_type; /* 0=classic, 1=enhanced, 2=auto */
    uint8_t data_len; /* 0=auto, 1-8 otherwise */
);

PACKED(lin_mode_resp_t,
    uint8_t mode; /* 0=commander, 1=responder */
);

PACKED(lin_status_resp_t, // status of responder mode sent frames (clear on read)
    uint8_t id; /* >0x3F=invalid (no recent frame sent) */
    int16_t retcode; /* 0=success */
);

PACKED(lin_write_req_t,
    uint8_t id;
    uint8_t checksum_type; /* 0=classic, 1=enhanced */
    uint8_t data[];
);

PACKED(lin_read_req_t,
    uint8_t id; // ignored in responder mode
);

PACKED(lin_read_resp_t, // empty response if none to read
    uint16_t num_left;
    uint8_t  id;
    uint8_t  checksum_type; /* 0=classic, 1=enhanced */
    uint8_t  data[];
);

/* Function indices */
#define LIN_SET_MODE_ID    0
#define LIN_SET_RATE_ID    1
#define LIN_SET_FILTER_ID  2
#define LIN_MODE_ID        3
#define LIN_STATUS_ID      4
#define LIN_WRITE_ID       5
#define LIN_READ_ID        6

#endif // JABI_PERIPHERALS_LIN_H
