#include "client.h"

/* Metadata */
std::string JABIClient::get_serial() {
    GetSerialRequest req;
    GetSerialResponse resp;
    ClientContext ctx;

    Status status = stub->get_serial(&ctx, req, &resp);

    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.sn();
}

int JABIClient::get_num_inst(int periph_id) {
    GetNumInstRequest req;
    GetNumInstResponse resp;
    ClientContext ctx;

    req.set_periph_id(periph_id);
    Status status = stub->get_num_inst(&ctx, req, &resp);

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
