#include <cstring>
#include <sstream>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/lin.h>

#define LIN_MAX_LEN 8

LINMessage::LINMessage()
:
    id(0), type(LINChecksum::CLASSIC)
{}

LINMessage::LINMessage(int id, std::vector<uint8_t> data, LINChecksum type)
:
    id(id), type(type), data(data)
{}

std::ostream &operator<<(std::ostream &os, LINStatus const &m) {
    std::stringstream s;
    s << std::hex << std::showbase << "LINStatus(id=" << m.id;
    s << ",success=" << (m.success ? "true" : "false") << ")";
    return os << s.str();
}

std::ostream &operator<<(std::ostream &os, LINMessage const &m) {
    std::stringstream s;
    s << std::hex << std::showbase << "LINMessage(id=" << m.id;
    s << ",type=";
    switch (m.type) {
        case LINChecksum::CLASSIC:  s << "classic";  break;
        case LINChecksum::ENHANCED: s << "enhanced"; break;
        case LINChecksum::AUTO:     s << "auto";     break;
        default:                    s << "unknown";  break;
    }
    s << ",data={";
    for (auto i : m.data) { s << static_cast<int>(i) << ","; }
    s << "})";
    return os << s.str();
}

void Device::lin_set_mode(LINMode mode, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_SET_MODE_ID,
            .payload_len = sizeof(lin_set_mode_j_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(lin_set_mode_j_req_t), 0),
    };

    auto args = reinterpret_cast<lin_set_mode_j_req_t*>(req.payload.data());
    args->mode = static_cast<uint8_t>(mode);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::lin_set_rate(int bitrate, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_SET_RATE_ID,
            .payload_len = sizeof(lin_set_rate_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(lin_set_rate_req_t), 0),
    };

    auto args = reinterpret_cast<lin_set_rate_req_t*>(req.payload.data());
    args->bitrate = htole<uint32_t>(bitrate);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::lin_set_filter(int id, int len, LINChecksum type, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_SET_FILTER_ID,
            .payload_len = sizeof(lin_set_filter_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(lin_set_filter_req_t), 0),
    };

    auto args = reinterpret_cast<lin_set_filter_req_t*>(req.payload.data());
    args->id = id;
    args->checksum_type = static_cast<uint8_t>(type);
    args->data_len = len;

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

LINMode Device::lin_mode(int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_MODE_ID,
            .payload_len = 0,
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != sizeof(lin_mode_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }
    auto ret = reinterpret_cast<lin_mode_resp_t*>(resp.payload.data());

    return static_cast<LINMode>(ret->mode);
}

LINStatus Device::lin_status(int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_STATUS_ID,
            .payload_len = 0,
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != sizeof(lin_status_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }
    auto ret = reinterpret_cast<lin_status_resp_t*>(resp.payload.data());
    ret->retcode = letoh<int16_t>(ret->retcode);

    LINStatus status = {
        .id = ret->id,
        .success = ret->retcode == 0,
    };
    return status;
}

void Device::lin_write(LINMessage msg, int idx) {
    if (msg.data.size() > LIN_MAX_LEN) {
        throw std::runtime_error("data too long");
    }

    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_WRITE_ID,
            .payload_len = sizeof(lin_write_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(lin_write_req_t), 0),
    };

    auto args = reinterpret_cast<lin_write_req_t*>(req.payload.data());
    args->id            = msg.id;
    args->checksum_type = static_cast<uint8_t>(msg.type);
    for (auto c : msg.data) {
        req.payload.push_back(c);
    }
    req.msg.payload_len = static_cast<uint16_t>(req.payload.size());

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

int Device::lin_read(LINMessage &msg, int id, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_LIN_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = LIN_READ_ID,
            .payload_len = sizeof(lin_read_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(lin_read_req_t), 0),
    };

    auto args = reinterpret_cast<lin_read_req_t*>(req.payload.data());
    args->id = id;

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() == 0) {
        return -1; // empty buffer, no message returned
    }
    if (resp.payload.size() < sizeof(lin_read_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = reinterpret_cast<lin_read_resp_t*>(resp.payload.data());
    ret->num_left = letoh<uint16_t>(ret->num_left);

    size_t data_len = resp.payload.size() - sizeof(lin_read_resp_t);
    if (data_len > LIN_MAX_LEN) {
        throw std::runtime_error("unexpected payload length");
    }

    msg.id   = ret->id;
    msg.type = static_cast<LINChecksum>(ret->checksum_type);
    msg.data = std::vector<uint8_t>(data_len, 0);
    memcpy(msg.data.data(), ret->data, data_len);
    return ret->num_left;
}

};
