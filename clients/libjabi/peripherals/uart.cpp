#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/uart.h>

void Device::uart_set_config(int baud, int data_bits,
        UARTParity parity, UARTStop stop, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_UART_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = UART_SET_CONFIG_ID,
            .payload_len = sizeof(uart_set_config_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(uart_set_config_req_t), 0),
    };

    auto args = reinterpret_cast<uart_set_config_req_t*>(req.payload.data());
    args->baud      = htole<uint32_t>(baud);
    args->data_bits = static_cast<uint8_t>(data_bits);
    args->parity    = static_cast<uint8_t>(parity);
    args->stop_bits = static_cast<uint8_t>(stop);

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::uart_write(std::vector<uint8_t> data, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_UART_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = UART_WRITE_ID,
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

std::vector<uint8_t> Device::uart_read(size_t len, int idx) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_UART_ID,
            .periph_idx  = static_cast<uint16_t>(idx),
            .periph_fn   = UART_READ_ID,
            .payload_len = sizeof(uart_read_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(uart_read_req_t)),
    };

    auto args = reinterpret_cast<uart_read_req_t*>(req.payload.data());
    args->data_len = htole<uint16_t>(static_cast<uint16_t>(len));

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() > len) {
        throw std::runtime_error("unexpected payload length");
    }
    return resp.payload;
}

};
