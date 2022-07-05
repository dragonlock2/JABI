#include <iostream>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "jabi.grpc.pb.h"

/* Client designed to provide the same interface as <libjabi/device.h>.
 * Most of the code is straight up copied 🔥 so the only dependency is jabi.proto.
 */

namespace jabi {

/* Metadata */
enum class InstID {
    METADATA = JABI::InstID::METADATA,
    CAN      = JABI::InstID::CAN,
    I2C      = JABI::InstID::I2C,
    GPIO     = JABI::InstID::GPIO,
};

/* CAN */
enum class CANMode {
    NORMAL     = JABI::CANMode::NORMAL,
    LOOPBACK   = JABI::CANMode::LOOPBACK,
    LISTENONLY = JABI::CANMode::LISTENONLY,
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
    STANDARD  = JABI::I2CFreq::STANDARD,  // 100kHz
    FAST      = JABI::I2CFreq::FAST,      // 400kHz
    FAST_PLUS = JABI::I2CFreq::FAST_PLUS, // 1MHz
    HIGH      = JABI::I2CFreq::HIGH,      // 3.4MHz
    ULTRA     = JABI::I2CFreq::ULTRA,     // 5MHz
};

/* GPIO */
enum class GPIODir {
    INPUT       = JABI::GPIODir::INPUT,
    OUTPUT      = JABI::GPIODir::OUTPUT,
    OPEN_DRAIN  = JABI::GPIODir::OPEN_DRAIN,
    OPEN_SOURCE = JABI::GPIODir::OPEN_SOURCE,
};

enum class GPIOPull {
    NONE = JABI::GPIOPull::NONE,
    UP   = JABI::GPIOPull::UP,
    DOWN = JABI::GPIOPull::DOWN,
    BOTH = JABI::GPIOPull::BOTH,
};

class gRPCDevice {
public:
    gRPCDevice(std::shared_ptr<grpc::Channel> channel)
        : stub(JABI::Device::NewStub(channel)) {}

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
    std::unique_ptr<JABI::Device::Stub> stub;
};

};
