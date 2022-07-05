#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/spi.h>

void Device::spi_set_freq(int freq, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_SET_FREQ_ID,
        .payload_len = sizeof(spi_set_freq_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<spi_set_freq_req_t*>(req.payload);
    args->freq = htole<uint32_t>(freq);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_set_mode(int mode, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_SET_MODE_ID,
        .payload_len = sizeof(spi_set_mode_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<spi_set_mode_req_t*>(req.payload);
    args->mode = mode;

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_set_bitorder(bool msb, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_SET_BITORDER_ID,
        .payload_len = sizeof(spi_set_bitorder_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<spi_set_bitorder_req_t*>(req.payload);
    args->order = msb;

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_write(std::vector<uint8_t> data, int idx) {
    if (data.size() > REQ_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("data too long");
    }

    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_WRITE_ID,
        .payload_len = static_cast<uint16_t>(data.size()),
        .payload     = {0},
    };

    memcpy(req.payload, data.data(), data.size());

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

std::vector<uint8_t> Device::spi_read(size_t len, int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_READ_ID,
        .payload_len = sizeof(spi_read_j_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<spi_read_j_req_t*>(req.payload);
    args->data_len = htole<uint16_t>(static_cast<uint16_t>(len));

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != len) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = std::vector<uint8_t>(len, 0);
    memcpy(ret.data(), resp.payload, len);
    return ret;
}

std::vector<uint8_t> Device::spi_transceive(std::vector<uint8_t> data, int idx) {
    if (data.size() > REQ_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("data too long");
    }

    iface_req_t req = {
        .periph_id   = PERIPH_SPI_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = SPI_TRANSCEIVE_ID,
        .payload_len = static_cast<uint16_t>(data.size()),
        .payload     = {0},
    };

    memcpy(req.payload, data.data(), data.size());

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != data.size()) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = std::vector<uint8_t>(data.size(), 0);
    memcpy(ret.data(), resp.payload, data.size());
    return ret;
}

};
