#ifndef JABI_PERIPHERALS_I2C_H
#define JABI_PERIPHERALS_I2C_H

#include <jabi/interfaces.h>

/*
 * Arbitrary frequencies is not easy w/ Zephyr :(
 * 0 = 100kHz (STANDARD)
 * 1 = 400kHz (FAST)
 * 2 = 1MHz   (FAST_PLUS)
 * 3 = 3.4MHz (HIGH)
 * 4 = 5MHz   (ULTRA)
 */
PACKED(i2c_set_freq_req_t,
    uint8_t preset;
);

PACKED(i2c_write_j_req_t,
    uint16_t addr;
    uint8_t  data[];
);

PACKED(i2c_read_j_req_t,
    uint16_t addr;
    uint16_t data_len;
);

typedef uint8_t i2c_read_j_resp_t;

// writes, then reads using restart
PACKED(i2c_transceive_req_t,
    uint16_t addr;
    uint16_t data_len;
    uint8_t data[];
);

typedef uint8_t i2c_transceive_resp_t;

/* Function indices */
#define I2C_SET_FREQ_ID   0
#define I2C_WRITE_ID      1
#define I2C_READ_ID       2
#define I2C_TRANSCEIVE_ID 3

#endif // JABI_PERIPHERALS_I2C_H
