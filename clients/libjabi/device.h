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

class Device {
public:
    /* Metadata */
    std::string serial();
    int num_inst(InstID id);
    std::string echo(std::string str);

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

    /* GPIO */
    void gpio_set_mode(int idx, GPIODir dir=GPIODir::INPUT,
        GPIOPull pull=GPIOPull::NONE, bool init_val=false);
    void gpio_write(int idx, bool val);
    bool gpio_read(int idx);

private:
    Device(std::shared_ptr<Interface> i) : interface(i) {}

    std::shared_ptr<Interface> interface;

    friend class Interface;
};

};

#endif // LIBJABI_DEVICE_H
