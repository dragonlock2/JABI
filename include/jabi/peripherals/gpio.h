#ifndef JABI_PERIPHERALS_GPIO_H
#define JABI_PERIPHERALS_GPIO_H

#include <jabi/interfaces.h>

PACKED(gpio_set_mode_req_t,
    uint8_t direction; /* 0=input, 1=output, 2=open drain, 3=open source */
    uint8_t pull;      /* 0=none, 1=up, 2=down, 3=both */
    uint8_t init_val;  /* 0=low, 1=high */
);

PACKED(gpio_write_req_t,
    uint8_t val;
);

PACKED(gpio_read_resp_t,
    uint8_t val;
);

/* Function indices */
#define GPIO_SET_MODE_ID 0
#define GPIO_WRITE_ID    1
#define GPIO_READ_ID     2

#endif // JABI_PERIPHERALS_GPIO_H
