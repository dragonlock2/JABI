#include "server.h"

/* Metadata */
Status JABIServiceImpl::serial(ServerContext*,
        const SerialRequest*, SerialResponse* resp) {
    try {
        resp->set_sn(dev->serial());
        return Status::OK;
    } catch(const std::runtime_error &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return Status::CANCELLED;
    }
}

Status JABIServiceImpl::num_inst(ServerContext*,
        const NumInstRequest* req, NumInstResponse* resp) {
    try {
        resp->set_num_idx(dev->num_inst(req->periph_id()));
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
