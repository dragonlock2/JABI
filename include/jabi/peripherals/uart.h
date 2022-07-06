#ifndef JABI_PERIPHERALS_UART_H
#define JABI_PERIPHERALS_UART_H

#include <jabi/interfaces.h>

PACKED(uart_set_config_req_t,
    uint32_t baud;
    uint8_t  data_bits; /* 5, 6, 7, 8, or 9 bits */
    uint8_t  parity;    /* 0=none, 1=odd, 2=even, 3=mark, 4=space */
    uint8_t  stop_bits; /* 0=0.5, 1=1, 2=1.5, 3=2 */
);

typedef uint8_t uart_write_req_t;

PACKED(uart_read_req_t,
    uint16_t data_len;
);

typedef uint8_t uart_read_resp_t;

/* Function indices */
#define UART_SET_CONFIG_ID 0
#define UART_WRITE_ID      1
#define UART_READ_ID       2

#endif // JABI_PERIPHERALS_UART_H
