#ifndef JABI_PERIPHERALS_SPI_H
#define JABI_PERIPHERALS_SPI_H

#include <jabi/interfaces.h>

PACKED(spi_set_freq_req_t,
    uint32_t freq; // Hz
);

/* Following Wikipedia,
 * mode=0 -> CPOL=0, CPHA=0 (MODE0)
 * mode=1 -> CPOL=0, CPHA=1 (MODE1)
 * mode=2 -> CPOL=1, CPHA=0 (MODE2)
 * mode=3 -> CPOL=1, CPHA=1 (MODE3)
 */
PACKED(spi_set_mode_req_t,
    uint8_t mode;
);

PACKED(spi_set_bitorder_req_t,
    uint8_t order; /* 0=LSB, 1=MSB */
);

typedef uint8_t spi_write_j_req_t;

PACKED(spi_read_j_req_t,
    uint16_t data_len;
);

typedef uint8_t spi_read_j_resp_t;

typedef uint8_t spi_transceive_j_req_t;
typedef uint8_t spi_transceive_j_resp_t;

/* Function indices */
#define SPI_SET_FREQ_ID     0
#define SPI_SET_MODE_ID     1
#define SPI_SET_BITORDER_ID 2
#define SPI_WRITE_ID        3
#define SPI_READ_ID         4
#define SPI_TRANSCEIVE_ID   5

#endif // JABI_PERIPHERALS_SPI_H
