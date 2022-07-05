#include <sstream>

#include "client.h"

using grpc::ClientContext;
using grpc::Status;

using google::protobuf::Empty;
using google::protobuf::StringValue;
using google::protobuf::UInt32Value;
using google::protobuf::BytesValue;
using google::protobuf::BoolValue;

namespace jabi {

/* Metadata */
std::string gRPCDevice::serial() {
    Empty req;
    StringValue resp;
    ClientContext ctx;
    Status status = stub->serial(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.value();
}

int gRPCDevice::num_inst(InstID id) {
    JABI::NumInstRequest req;
    UInt32Value resp;
    ClientContext ctx;
    req.set_id(static_cast<JABI::InstID>(id));
    Status status = stub->num_inst(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.value();
}

std::string gRPCDevice::echo(std::string str) {
    StringValue req;
    StringValue resp;
    ClientContext ctx;
    req.set_value(str);
    Status status = stub->echo(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.value();
}

/* CAN */
CANMessage::CANMessage()
:
    id(0), ext(false), fd(false), brs(false), rtr(false)
{}

CANMessage::CANMessage(int id, int req_len, bool fd, bool brs)
:
    id(id), ext(id & ~0x7FF), fd(fd), brs(brs), rtr(true), data(req_len, 0)
{}

CANMessage::CANMessage(int id, std::vector<uint8_t> data, bool fd, bool brs)
:
    id(id), ext(id & ~0x7FF), fd(fd), brs(brs), rtr(false), data(data)
{}

std::ostream &operator<<(std::ostream &os, CANMessage const &m) {
    std::stringstream s;
    s << std::hex << std::showbase << "CANMessage(";
    s <<  "id="  << m.id  << ",ext=" << m.ext << ",fd=" << m.fd;
    s << ",brs=" << m.brs << ",rtr=" << m.rtr;
    if (m.rtr) {
        s << ",data.size()=" << m.data.size();
    } else {
        s << ",data={";
        for (auto i : m.data) { s << static_cast<int>(i) << ","; }
        s << "}";
    }
    s << ")";
    return os << s.str();
}

void gRPCDevice::can_set_filter(int id, int id_mask, bool rtr, bool rtr_mask, int idx) {
    JABI::CANSetFilterRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_id(id);
    req.set_id_mask(id_mask);
    req.set_rtr(rtr);
    req.set_rtr_mask(rtr_mask);
    req.set_idx(idx);
    Status status = stub->can_set_filter(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

void gRPCDevice::can_set_rate(int bitrate, int bitrate_data, int idx) {
    JABI::CANSetRateRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_bitrate(bitrate);
    req.set_bitrate_data(bitrate_data);
    req.set_idx(idx);
    Status status = stub->can_set_rate(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

void gRPCDevice::can_set_mode(CANMode mode, int idx) {
    JABI::CANSetModeRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_mode(static_cast<JABI::CANMode>(mode));
    req.set_idx(idx);
    Status status = stub->can_set_mode(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

CANState gRPCDevice::can_state(int idx) {
    JABI::Index req;
    JABI::CANStateResponse resp;
    ClientContext ctx;
    req.set_idx(idx);
    Status status = stub->can_state(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    CANState s = {
        .state  = static_cast<int>(resp.state()),
        .tx_err = static_cast<int>(resp.tx_err()),
        .rx_err = static_cast<int>(resp.rx_err()),
    };
    return s;
}

void gRPCDevice::can_write(CANMessage msg, int idx) {
    JABI::CANWriteRequest req;
    Empty resp;
    ClientContext ctx;
    JABI::CANMessage* m = new JABI::CANMessage();
    m->set_id(msg.id);
    m->set_ext(msg.ext);
    m->set_fd(msg.fd);
    m->set_brs(msg.brs);
    m->set_rtr(msg.rtr);
    m->set_data(std::string(msg.data.begin(), msg.data.end()));
    req.set_allocated_msg(m);
    req.set_idx(idx);
    Status status = stub->can_write(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

int gRPCDevice::can_read(CANMessage &msg, int idx) {
    JABI::Index req;
    JABI::CANReadResponse resp;
    ClientContext ctx;
    req.set_idx(idx);
    Status status = stub->can_read(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    msg.id   = resp.msg().id();
    msg.ext  = resp.msg().ext();
    msg.fd   = resp.msg().fd();
    msg.brs  = resp.msg().brs();
    msg.rtr  = resp.msg().rtr();
    auto s   = resp.msg().data();
    msg.data = std::vector<uint8_t>(s.begin(), s.end());
    return resp.num_left();
}

/* I2C */
void gRPCDevice::i2c_set_freq(I2CFreq preset, int idx) {
    JABI::I2CSetFreqRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_preset(static_cast<JABI::I2CFreq>(preset));
    req.set_idx(idx);
    Status status = stub->i2c_set_freq(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

void gRPCDevice::i2c_write(int addr, std::vector<uint8_t> data, int idx) {
    JABI::I2CWriteRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_addr(addr);
    req.set_data(std::string(data.begin(), data.end()));
    req.set_idx(idx);
    Status status = stub->i2c_write(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

std::vector<uint8_t> gRPCDevice::i2c_read(int addr, size_t len, int idx) {
    JABI::I2CReadRequest req;
    BytesValue resp;
    ClientContext ctx;
    req.set_addr(addr);
    req.set_len(len);
    req.set_idx(idx);
    Status status = stub->i2c_read(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return std::vector<uint8_t>(resp.value().begin(), resp.value().end());
}

/* GPIO */
void gRPCDevice::gpio_set_mode(int idx, GPIODir dir, GPIOPull pull, bool init_val) {
    JABI::GPIOSetModeRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_idx(idx);
    req.set_dir(static_cast<JABI::GPIODir>(dir));
    req.set_pull(static_cast<JABI::GPIOPull>(pull));
    req.set_init_val(init_val);
    Status status = stub->gpio_set_mode(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

void gRPCDevice::gpio_write(int idx, bool val) {
    JABI::GPIOWriteRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_idx(idx);
    req.set_val(val);
    Status status = stub->gpio_write(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

bool gRPCDevice::gpio_read(int idx) {
    JABI::Index req;
    BoolValue resp;
    ClientContext ctx;
    req.set_idx(idx);
    Status status = stub->gpio_read(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
    return resp.value();
}

/* PWM */
void gRPCDevice::pwm_write(int idx, double pulsewidth, double period) {
    JABI::PWMWriteRequest req;
    Empty resp;
    ClientContext ctx;
    req.set_idx(idx);
    req.set_pulsewidth(pulsewidth);
    req.set_period(period);
    Status status = stub->pwm_write(&ctx, req, &resp);
    if (!status.ok()) {
        throw std::runtime_error("fail");
    }
}

};
