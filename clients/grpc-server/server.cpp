#include "server.h"

/* Metadata */
Status JABIServiceImpl::get_serial(ServerContext*,
        const GetSerialRequest*, GetSerialResponse* resp) {
    try {
        resp->set_sn(dev->get_serial());
        return Status::OK;
    } catch(const std::runtime_error &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return Status::CANCELLED;
    }
}

Status JABIServiceImpl::get_num_inst(ServerContext*,
        const GetNumInstRequest* req, GetNumInstResponse* resp) {
    try {
        resp->set_num_idx(dev->get_num_inst(req->periph_id()));
        return Status::OK;
    } catch(const std::runtime_error &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return Status::CANCELLED;
    }
}

Status JABIServiceImpl::echo(ServerContext*,
        const EchoRequest* req, EchoResponse* resp) {
    try {
        resp->set_msg(dev->echo(req->msg()));
        return Status::OK;
    } catch(const std::runtime_error &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return Status::CANCELLED;
    }
}
