#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/i2c.h>

void Device::i2c_set_freq(I2CFreq preset, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_I2C_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = I2C_SET_FREQ_ID,
        .payload_len = sizeof(i2c_set_freq_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<i2c_set_freq_req_t*>(req.payload);
    args->preset = static_cast<uint8_t>(preset);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::i2c_write(int addr, std::vector<uint8_t> data, int idx) {
    if ((sizeof(i2c_write_j_req_t) + data.size()) > REQ_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("data too long");
    }

    iface_req_t req = {
        .periph_id   = PERIPH_I2C_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = I2C_WRITE_ID,
        .payload_len = static_cast<uint16_t>(sizeof(i2c_write_j_req_t) + data.size()),
        .payload     = {0},
    };

    auto args = reinterpret_cast<i2c_write_j_req_t*>(req.payload);
    args->addr = htole<uint16_t>(addr);
    args->data_len = htole<uint16_t>(data.size());
    memcpy(args->data, data.data(), data.size());

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

std::vector<uint8_t> Device::i2c_read(int addr, size_t len, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_I2C_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = I2C_READ_ID,
        .payload_len = sizeof(i2c_read_j_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<i2c_read_j_req_t*>(req.payload);
    args->addr = htole<uint16_t>(addr);
    args->data_len = htole<uint16_t>(len);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != len) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = std::vector<uint8_t>(len, 0);
    memcpy(ret.data(), resp.payload, len);
    return ret;
}

};
