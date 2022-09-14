#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/i2c.h>

void Device::i2c_set_freq(I2CFreq preset, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_I2C_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = I2C_SET_FREQ_ID,
            .payload_len = sizeof(i2c_set_freq_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(i2c_set_freq_req_t), 0),
    };

    auto args = reinterpret_cast<i2c_set_freq_req_t*>(req.payload.data());
    args->preset = static_cast<uint8_t>(preset);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::i2c_write(int addr, std::vector<uint8_t> data, int idx) {
    if (sizeof(i2c_write_j_req_t) + data.size() > interface->get_req_max_size()) {
        throw std::runtime_error("data too long");
    }
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_I2C_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = I2C_WRITE_ID,
            .payload_len = static_cast<uint16_t>(sizeof(i2c_write_j_req_t) + data.size()),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(i2c_write_j_req_t) + data.size(), 0),
    };

    auto args = reinterpret_cast<i2c_write_j_req_t*>(req.payload.data());
    args->addr = htole<uint16_t>(static_cast<uint16_t>(addr));
    memcpy(args->data, data.data(), data.size());

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

std::vector<uint8_t> Device::i2c_read(int addr, size_t len, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_I2C_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = I2C_READ_ID,
            .payload_len = sizeof(i2c_read_j_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(i2c_read_j_req_t), 0),
    };

    auto args = reinterpret_cast<i2c_read_j_req_t*>(req.payload.data());
    args->addr = htole<uint16_t>(static_cast<uint16_t>(addr));
    args->data_len = htole<uint16_t>(static_cast<uint16_t>(len));

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != len) {
        throw std::runtime_error("unexpected payload length");
    }

    return resp.payload;
}

std::vector<uint8_t> Device::i2c_transceive(int addr, std::vector<uint8_t> data, size_t read_len, int idx) {
    if (sizeof(i2c_transceive_req_t) + data.size() > interface->get_req_max_size()) {
        throw std::runtime_error("data too long");
    }
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_I2C_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = I2C_TRANSCEIVE_ID,
            .payload_len = static_cast<uint16_t>(sizeof(i2c_transceive_req_t) + data.size()),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(i2c_transceive_req_t) + data.size(), 0),
    };

    auto args = reinterpret_cast<i2c_transceive_req_t*>(req.payload.data());
    args->addr = htole<uint16_t>(static_cast<uint16_t>(addr));
    args->data_len = htole<uint16_t>(static_cast<uint16_t>(read_len));
    memcpy(args->data, data.data(), data.size());

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != read_len) {
        throw std::runtime_error("unexpected payload length");
    }

    return resp.payload;
}

};
