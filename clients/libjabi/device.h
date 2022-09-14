#ifndef LIBJABI_DEVICE_H
#define LIBJABI_DEVICE_H

#include <iostream>
#include <memory>
#include <vector>

namespace jabi {

#include <jabi/peripherals.h>

class Interface;

/* Metadata */
enum class InstID {
    METADATA = PERIPH_METADATA_ID,
    CAN      = PERIPH_CAN_ID,
    I2C      = PERIPH_I2C_ID,
    GPIO     = PERIPH_GPIO_ID,
    PWM      = PERIPH_PWM_ID,
    ADC      = PERIPH_ADC_ID,
    DAC      = PERIPH_DAC_ID,
    SPI      = PERIPH_SPI_ID,
    UART     = PERIPH_UART_ID,
    LIN      = PERIPH_LIN_ID,
};

/* CAN */
enum class CANMode {
    NORMAL     = 0,
    LOOPBACK   = 1,
    LISTENONLY = 2,
};

struct CANState {
    int state;
    int tx_err;
    int rx_err;
};

struct CANMessage {
    int  id;
    bool ext;
    bool fd;
    bool brs;
    bool rtr;
    std::vector<uint8_t> data;

    CANMessage();
    CANMessage(int id, int req_len, bool fd=false, bool brs=false);
    CANMessage(int id, std::vector<uint8_t> data, bool fd=false, bool brs=false);
};

std::ostream &operator<<(std::ostream &os, CANMessage const &m);

/* I2C */
enum class I2CFreq {
    STANDARD  = 0, // 100kHz
    FAST      = 1, // 400kHz
    FAST_PLUS = 2, // 1MHz
    HIGH      = 3, // 3.4MHz
    ULTRA     = 4, // 5MHz
};

/* GPIO */
enum class GPIODir {
    INPUT       = 0,
    OUTPUT      = 1,
    OPEN_DRAIN  = 2,
    OPEN_SOURCE = 3,
};

enum class GPIOPull {
    NONE = 0,
    UP   = 1,
    DOWN = 2,
    BOTH = 3,
};

/* UART */
enum class UARTParity {
    NONE  = 0,
    ODD   = 1,
    EVEN  = 2,
    MARK  = 3,
    SPACE = 4,
};

enum class UARTStop {
    B0_5 = 0,
    B1   = 1,
    B1_5 = 2,
    B2   = 3,
};

/* LIN */
enum class LINMode {
    COMMANDER = 0,
    RESPONDER = 1,
};

enum class LINChecksum {
    CLASSIC  = 0,
    ENHANCED = 1,
    AUTO     = 2,
};

struct LINStatus {
    int  id;
    bool success;
};

struct LINMessage {
    int id;
    LINChecksum type;
    std::vector<uint8_t> data;

    LINMessage();
    LINMessage(int id, std::vector<uint8_t> data, LINChecksum type=LINChecksum::ENHANCED);
};

std::ostream &operator<<(std::ostream &os, LINStatus const &m);
std::ostream &operator<<(std::ostream &os, LINMessage const &m);

class Device {
public:
    /* Metadata */
    std::string serial();
    int num_inst(InstID id);
    std::string echo(std::string str);
    size_t req_max_size();
    size_t resp_max_size();

    /* CAN */
    void can_set_filter(int id, int id_mask, bool rtr, bool rtr_mask, int idx=0);
    void can_set_rate(int bitrate, int bitrate_data, int idx=0);
    void can_set_mode(CANMode mode, int idx=0);
    CANState can_state(int idx=0);
    void can_write(CANMessage msg, int idx=0);
    int can_read(CANMessage &msg, int idx=0);

    /* I2C */
    void i2c_set_freq(I2CFreq preset, int idx=0);
    void i2c_write(int addr, std::vector<uint8_t> data, int idx=0);
    std::vector<uint8_t> i2c_read(int addr, size_t len, int idx=0);
    std::vector<uint8_t> i2c_transceive(int addr, std::vector<uint8_t> data, size_t read_len, int idx=0);

    /* GPIO */
    void gpio_set_mode(int idx, GPIODir dir=GPIODir::INPUT,
        GPIOPull pull=GPIOPull::NONE, bool init_val=false);
    void gpio_write(int idx, bool val);
    bool gpio_read(int idx);

    /* PWM */
    void pwm_write(int idx, double pulsewidth, double period); // seconds

    /* ADC */
    int adc_read(int idx); // mV

    /* DAC */
    void dac_write(int idx, int mV);

    /* SPI */
    void spi_set_freq(int freq, int idx=0);
    void spi_set_mode(int mode, int idx=0);
    void spi_set_bitorder(bool msb, int idx=0);
    void spi_write(std::vector<uint8_t> data, int idx=0);
    std::vector<uint8_t> spi_read(size_t len, int idx=0);
    std::vector<uint8_t> spi_transceive(std::vector<uint8_t> data, int idx=0);

    /* UART */
    void uart_set_config(int baud=115200, int data_bits=8,
        UARTParity parity=UARTParity::NONE, UARTStop stop=UARTStop::B1, int idx=0);
    void uart_write(std::vector<uint8_t> data, int idx=0);
    std::vector<uint8_t> uart_read(size_t len, int idx=0);

    /* LIN */
    void lin_set_mode(LINMode mode, int idx=0);
    void lin_set_rate(int bitrate, int idx=0);
    void lin_set_filter(int id, int len=0, LINChecksum type=LINChecksum::AUTO, int idx=0);
    LINMode lin_mode(int idx=0);
    LINStatus lin_status(int idx=0);
    void lin_write(LINMessage msg, int idx=0);
    int lin_read(LINMessage &msg, int id=0xFF, int idx=0);

private:
    Device(std::shared_ptr<Interface> i) : interface(i) {}

    std::shared_ptr<Interface> interface;

    friend class Interface;
};

};

#endif // LIBJABI_DEVICE_H
