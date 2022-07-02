#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/can.h>

CANMessage::CANMessage()
:
    id(0), ext(false), fd(false), brs(false), rtr(false), len(0), data({})
{}

CANMessage::CANMessage(int id, int req_len, bool fd, bool brs)
:
    id(id), ext(id & ~0x7FF), fd(fd), brs(brs), rtr(true), len(req_len), data({})
{}

CANMessage::CANMessage(int id, std::vector<char> data, bool fd, bool brs)
:
    id(id), ext(id & ~0x7FF), fd(fd), brs(brs), rtr(false), len(data.size())
{
    if (data.size() > this->data.size()) {
        throw std::runtime_error("input data too long");
    }
    std::copy(data.begin(), data.end(), this->data.begin());
    for (auto i = data.size(); i < this->data.size(); i++) {
        this->data[i] = 0;
    }
}

std::ostream &operator<<(std::ostream &os, CANMessage const &m) {
    os << "CANMessage(id=" << m.id << ",ext=" << m.ext << ",fd=" << m.fd;
    os << ",brs=" << m.brs << ",rtr=" << m.rtr << ",len=" << m.len;
    if (!m.rtr) {
        os << ",data={";
        for (auto i = 0; i < m.len-1; i++) {
            os << static_cast<int>(std::byte(m.data[i])) << ",";
        }
        os << static_cast<int>(std::byte(m.data[m.len-1])) << "}";
    }
    os << ")";
    return os;
}

void Device::can_set_filter(int id, int id_mask, bool rtr, bool rtr_mask, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_SET_FILTER_ID,
        .payload_len = sizeof(can_set_filter_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<can_set_filter_req_t*>(req.payload);
    args->id       = htole<uint32_t>(id);
    args->id_mask  = htole<uint32_t>(id_mask);
    args->rtr      = rtr;
    args->rtr_mask = rtr_mask;

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::can_set_rate(int bitrate, int bitrate_data, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_SET_RATE_ID,
        .payload_len = sizeof(can_set_rate_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<can_set_rate_req_t*>(req.payload);
    args->bitrate      = htole<uint32_t>(bitrate);
    args->bitrate_data = htole<uint32_t>(bitrate_data);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::can_set_mode(CANMode mode, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_SET_STYLE_ID,
        .payload_len = sizeof(can_set_style_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<can_set_style_req_t*>(req.payload);
    args->mode = static_cast<uint8_t>(mode);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

CANState Device::can_state(int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_STATE_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != sizeof(can_state_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }
    auto ret = reinterpret_cast<can_state_resp_t*>(req.payload);

    CANState state = {
        .state  = ret->state,
        .tx_err = ret->tx_err_cnt,
        .rx_err = ret->rx_err_cnt,
    };
    return state;
}

void Device::can_write(CANMessage msg, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_WRITE_ID,
        .payload_len = sizeof(can_write_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<can_write_req_t*>(req.payload);
    args->id       = htole<uint32_t>(msg.id);
    args->id_type  = msg.ext;
    args->fd       = msg.fd;
    args->brs      = msg.brs;
    args->rtr      = msg.rtr;
    args->data_len = std::min<size_t>(msg.data.size(), msg.len);
    if (!msg.rtr) {
        memcpy(args->data, msg.data.data(), args->data_len);
        req.payload_len += args->data_len;
    }

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

int Device::can_read(CANMessage &msg, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_CAN_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = CAN_READ_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len == 0) {
        return -1; // empty buffer, no message returned
    }
    if (resp.payload_len < sizeof(can_read_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = reinterpret_cast<can_read_resp_t*>(resp.payload);
    ret->num_left = letoh<uint16_t>(ret->num_left);
    ret->id       = letoh<uint32_t>(ret->id);

    if  ((ret->rtr && resp.payload_len != sizeof(can_read_resp_t)) ||
        (!ret->rtr && resp.payload_len != sizeof(can_read_resp_t) + ret->data_len) ||
          ret->data_len > msg.data.size()) {
        throw std::runtime_error("unexpected payload length");
    }

    msg.id  = ret->id;
    msg.ext = ret->id_type;
    msg.fd  = ret->fd;
    msg.brs = ret->brs;
    msg.rtr = ret->rtr;
    msg.len = ret->data_len;
    if (!ret->rtr) {
        memcpy(msg.data.data(), ret->data, ret->data_len);
    }
    return ret->num_left;
}

};
