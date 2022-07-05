#ifndef JABI_PERIPHERALS_ADC_H
#define JABI_PERIPHERALS_ADC_H

#include <jabi/interfaces.h>

PACKED(adc_read_j_resp_t,
    int32_t mv;
);

/* Function indices */
#define ADC_READ_ID 0

#endif // JABI_PERIPHERALS_ADC_H
