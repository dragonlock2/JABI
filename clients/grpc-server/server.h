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
    Status serial(ServerContext*, const SerialRequest*, SerialResponse* resp) override;
    Status num_inst(ServerContext*, const NumInstRequest* req, NumInstResponse* resp) override;
    Status echo(ServerContext*, const EchoRequest* req, EchoResponse* resp) override;

private:
    std::shared_ptr<jabi::Device> dev;
};
