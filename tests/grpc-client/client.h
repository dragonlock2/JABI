#include <iostream>

#include <grpcpp/grpcpp.h>
#include "jabi.grpc.pb.h"

#include <libjabi/device.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class JABIClient {
public:
    JABIClient(std::shared_ptr<Channel> channel)
        : stub(JABI::NewStub(channel)) {}

    /* Metadata */
    std::string get_serial();
    int get_num_inst(int periph_id);
    std::string echo(std::string str);

private:
    std::unique_ptr<JABI::Stub> stub;
};
