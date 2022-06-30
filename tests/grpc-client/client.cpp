#include "client.h"

/* Metadata */
std::string JABIClient::serial() {
    SerialRequest req;
    SerialResponse resp;
    ClientContext ctx;

    Status status = stub->serial(&ctx, req, &resp);

    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.sn();
}

int JABIClient::num_inst(int periph_id) {
    NumInstRequest req;
    NumInstResponse resp;
    ClientContext ctx;

    req.set_periph_id(periph_id);
    Status status = stub->num_inst(&ctx, req, &resp);

    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.num_idx();
}

std::string JABIClient::echo(std::string str) {
    EchoRequest req;
    EchoResponse resp;
    ClientContext ctx;

    req.set_msg(str);
    Status status = stub->echo(&ctx, req, &resp);

    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.msg();
}
