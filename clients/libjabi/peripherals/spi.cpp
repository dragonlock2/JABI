#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/spi.h>

void Device::spi_set_freq(int freq, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_SET_FREQ_ID,
            .payload_len = sizeof(spi_set_freq_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(spi_set_freq_req_t), 0),
    };

    auto args = reinterpret_cast<spi_set_freq_req_t*>(req.payload.data());
    args->freq = htole<uint32_t>(freq);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_set_mode(int mode, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_SET_MODE_ID,
            .payload_len = sizeof(spi_set_mode_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(spi_set_mode_req_t), 0),
    };

    auto args = reinterpret_cast<spi_set_mode_req_t*>(req.payload.data());
    args->mode = static_cast<uint8_t>(mode);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_set_bitorder(bool msb, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_SET_BITORDER_ID,
            .payload_len = sizeof(spi_set_bitorder_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(spi_set_bitorder_req_t), 0),
    };

    auto args = reinterpret_cast<spi_set_bitorder_req_t*>(req.payload.data());
    args->order = msb;

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::spi_write(std::vector<uint8_t> data, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_WRITE_ID,
            .payload_len = static_cast<uint16_t>(data.size()),
            .payload     = {0},
        },
        .payload = data,
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

std::vector<uint8_t> Device::spi_read(size_t len, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_READ_ID,
            .payload_len = sizeof(spi_read_j_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(spi_read_j_req_t)),
    };

    auto args = reinterpret_cast<spi_read_j_req_t*>(req.payload.data());
    args->data_len = htole<uint16_t>(static_cast<uint16_t>(len));

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != len) {
        throw std::runtime_error("unexpected payload length");
    }
    return resp.payload;
}

std::vector<uint8_t> Device::spi_transceive(std::vector<uint8_t> data, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_SPI_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = SPI_TRANSCEIVE_ID,
            .payload_len = static_cast<uint16_t>(data.size()),
            .payload     = {0},
        },
        .payload = data,
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != data.size()) {
        throw std::runtime_error("unexpected payload length");
    }
    return resp.payload;
}

};
