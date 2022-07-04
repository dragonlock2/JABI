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

private:
    std::unique_ptr<JABI::Device::Stub> stub;
};

};
