#ifndef JABI_PERIPHERALS_PWM_H
#define JABI_PERIPHERALS_PWM_H

#include <jabi/interfaces.h>

PACKED(pwm_write_req_t,
    uint32_t pulsewidth; // ns
    uint32_t period; // ns
);

/* Function indices */
#define PWM_WRITE_ID 0

#endif // JABI_PERIPHERALS_PWM_H