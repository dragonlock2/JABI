#include "server.h"

#define CHECK_EXCEPT(c)                                  \
    try {                                                \
        c                                                \
        return Status::OK;                               \
    } catch(const std::runtime_error &e) {               \
        std::cerr << "error: " << e.what() << std::endl; \
        return Status::CANCELLED;                        \
    }

/* Metadata */
Status JABIServiceImpl::serial(ServerContext*, const Empty*, StringValue* resp) {
    CHECK_EXCEPT(
        resp->set_value(dev->serial());
    )
}

Status JABIServiceImpl::num_inst(ServerContext*, const NumInstRequest* req, UInt32Value* resp) {
    CHECK_EXCEPT(
        resp->set_value(dev->num_inst(static_cast<jabi::InstID>(req->id())));
    )
}

Status JABIServiceImpl::echo(ServerContext*, const StringValue* req, StringValue* resp) {
    CHECK_EXCEPT(
        resp->set_value(dev->echo(req->value()));
    )
}

/* CAN */
Status JABIServiceImpl::can_set_filter(ServerContext*, const CANSetFilterRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->can_set_filter(req->id(), req->id_mask(), req->rtr(), req->rtr_mask(), req->idx());
    )
}

Status JABIServiceImpl::can_set_rate(ServerContext*, const CANSetRateRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->can_set_rate(req->bitrate(), req->bitrate_data(), req->idx());
    )
}

Status JABIServiceImpl::can_set_mode(ServerContext*, const CANSetModeRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->can_set_mode(static_cast<jabi::CANMode>(req->mode()), req->idx());
    )
}

Status JABIServiceImpl::can_state(ServerContext*, const Index* req, CANStateResponse* resp) {
    CHECK_EXCEPT(
        jabi::CANState s = dev->can_state(req->idx());
        resp->set_state(s.state);
        resp->set_tx_err(s.tx_err);
        resp->set_rx_err(s.rx_err);
    )
}

Status JABIServiceImpl::can_write(ServerContext*, const CANWriteRequest* req, Empty*) {
    CHECK_EXCEPT(
        jabi::CANMessage m;
        m.id   = req->msg().id();
        m.ext  = req->msg().ext();
        m.fd   = req->msg().fd();
        m.brs  = req->msg().brs();
        m.rtr  = req->msg().rtr();
        auto s = req->msg().data();
        m.data = std::vector<uint8_t>(s.begin(), s.end());
        dev->can_write(m, req->idx());
    )
}

Status JABIServiceImpl::can_read(ServerContext*, const Index* req, CANReadResponse* resp) {
    CHECK_EXCEPT(
        jabi::CANMessage msg;
        resp->set_num_left(dev->can_read(msg, req->idx()));
        CANMessage* m = new CANMessage();
        m->set_id(msg.id);
        m->set_ext(msg.ext);
        m->set_fd(msg.fd);
        m->set_brs(msg.brs);
        m->set_rtr(msg.rtr);
        m->set_data(std::string(msg.data.begin(), msg.data.end()));
        resp->set_allocated_msg(m);
    )
}

/* I2C */
Status JABIServiceImpl::i2c_set_freq(ServerContext*, const I2CSetFreqRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->i2c_set_freq(static_cast<jabi::I2CFreq>(req->preset()), req->idx());
    )
}

Status JABIServiceImpl::i2c_write(ServerContext*, const I2CWriteRequest* req, Empty*) {
    CHECK_EXCEPT(
        auto s = req->data();
        dev->i2c_write(req->addr(), std::vector<uint8_t>(s.begin(), s.end()), req->idx());
    )
}

Status JABIServiceImpl::i2c_read(ServerContext*, const I2CReadRequest* req, BytesValue* resp) {
    CHECK_EXCEPT(
        auto v = dev->i2c_read(req->addr(), req->len(), req->idx());
        resp->set_value(std::string(v.begin(), v.end()));
    )
}

/* GPIO */
Status JABIServiceImpl::gpio_set_mode(ServerContext*, const GPIOSetModeRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->gpio_set_mode(req->idx(), static_cast<jabi::GPIODir>(req->dir()),
            static_cast<jabi::GPIOPull>(req->pull()), req->init_val());
    )
}

Status JABIServiceImpl::gpio_write(ServerContext*, const GPIOWriteRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->gpio_write(req->idx(), req->val());
    )
}

Status JABIServiceImpl::gpio_read(ServerContext*, const Index* req, BoolValue* resp) {
    CHECK_EXCEPT(
        resp->set_value(dev->gpio_read(req->idx()));
    )
}

/* PWM */
Status JABIServiceImpl::pwm_write(ServerContext*, const PWMWriteRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->pwm_write(req->idx(), req->pulsewidth(), req->period());
    )
}

/* ADC */
Status JABIServiceImpl::adc_read(ServerContext*, const Index* req, Int32Value* resp) {
    CHECK_EXCEPT(
        resp->set_value(dev->adc_read(req->idx()));
    )
}

/* DAC */
Status JABIServiceImpl::dac_write(ServerContext*, const DACWriteRequest* req, Empty*) {
    CHECK_EXCEPT(
        dev->dac_write(req->idx(), req->mv());
    )
}
