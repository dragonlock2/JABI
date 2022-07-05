#ifndef JABI_PERIPHERALS_DAC_H
#define JABI_PERIPHERALS_DAC_H

#include <jabi/interfaces.h>

PACKED(dac_write_req_t,
    int32_t mv;
);

/* Function indices */
#define DAC_WRITE_ID 0

#endif // JABI_PERIPHERALS_DAC_H
