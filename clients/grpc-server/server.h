#include <iostream>

#include <grpcpp/grpcpp.h>
#include "jabi.grpc.pb.h"

#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using google::protobuf::Empty;
using google::protobuf::StringValue;
using google::protobuf::UInt32Value;
using google::protobuf::BytesValue;

using namespace JABI;

class JABIServiceImpl final : public Device::Service {
public:
    JABIServiceImpl(std::shared_ptr<jabi::Device> dev) : dev(dev) {}

    /* Metadata */
    Status serial(ServerContext*, const Empty*, StringValue* resp) override;
    Status num_inst(ServerContext*, const NumInstRequest* req, UInt32Value* resp) override;
    Status echo(ServerContext*, const StringValue* req, StringValue* resp) override;

    /* CAN */
    Status can_set_filter(ServerContext*, const CANSetFilterRequest* req, Empty*) override;
    Status can_set_rate(ServerContext*, const CANSetRateRequest* req, Empty*) override;
    Status can_set_mode(ServerContext*, const CANSetModeRequest* req, Empty*) override;
    Status can_state(ServerContext*, const Index* req, CANStateResponse* resp) override;
    Status can_write(ServerContext*, const CANWriteRequest* req, Empty*) override;
    Status can_read(ServerContext*, const Index* req, CANReadResponse* resp) override;

    /* I2C */
    Status i2c_set_freq(ServerContext*, const I2CSetFreqRequest* req, Empty*) override;
    Status i2c_write(ServerContext*, const I2CWriteRequest* req, Empty*) override;
    Status i2c_read(ServerContext*, const I2CReadRequest* req, BytesValue* resp) override;

private:
    std::shared_ptr<jabi::Device> dev;
};
