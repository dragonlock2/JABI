#include <iostream>

#include <grpcpp/grpcpp.h>
#include "jabi.grpc.pb.h"

#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class JABIServiceImpl final : public JABI::Service {
public:
    JABIServiceImpl(std::shared_ptr<jabi::Device> dev) : dev(dev) {}

    /* Metadata */
    Status get_serial(ServerContext*, const GetSerialRequest*, GetSerialResponse* resp) override;
    Status get_num_inst(ServerContext*, const GetNumInstRequest* req, GetNumInstResponse* resp) override;
    Status echo(ServerContext*, const EchoRequest* req, EchoResponse* resp) override;

private:
    std::shared_ptr<jabi::Device> dev;
};
