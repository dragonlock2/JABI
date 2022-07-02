#ifndef JABI_PERIPHERALS_CAN_H
#define JABI_PERIPHERALS_CAN_H

#include <jabi/interfaces.h>

// struct bit fields not deterministic == more bytes :(

PACKED(can_set_filter_req_t,
    uint32_t id;
    uint32_t id_mask;
    uint8_t  rtr; /* 0=data frame, 1=remote request */
    uint8_t  rtr_mask;
);

PACKED(can_set_rate_req_t,
    uint32_t bitrate;
    uint32_t bitrate_data;
);

PACKED(can_set_style_req_t, // can_set_mode was taken :P
    uint8_t mode; /* 0=normal, 1=loopback, 2=listen only */
);

PACKED(can_state_resp_t,
    uint8_t state; // returns enum can_state
    uint8_t tx_err_cnt;
    uint8_t rx_err_cnt;
);

PACKED(can_write_req_t,
    uint32_t id;
    uint8_t  id_type;  /* 0=standard, 1=extended */
    uint8_t  fd;
    uint8_t  brs;
    uint8_t  rtr;      /* 0=data frame, 1=remote request */
    uint8_t  data_len;
    uint8_t  data[];
);

PACKED(can_read_resp_t, // empty response if none to read
    uint16_t num_left;
    uint32_t id;
    uint8_t  id_type;  /* 0=standard, 1=extended */
    uint8_t  fd;
    uint8_t  brs;
    uint8_t  rtr;      /* 0=data frame, 1=remote request */
    uint8_t  data_len;
    uint8_t  data[];
);

/* Function indices */
#define CAN_SET_FILTER_ID  0
#define CAN_SET_RATE_ID    1
#define CAN_SET_STYLE_ID   2
#define CAN_STATE_ID       3
#define CAN_WRITE_ID       4
#define CAN_READ_ID        5

#endif // JABI_PERIPHERALS_CAN_H
