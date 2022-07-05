#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/gpio.h>

void Device::gpio_set_mode(int idx, GPIODir dir, GPIOPull pull, bool init_val) {
    iface_req_t req = {
        .periph_id   = PERIPH_GPIO_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = GPIO_SET_MODE_ID,
        .payload_len = sizeof(gpio_set_mode_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<gpio_set_mode_req_t*>(req.payload);
    args->direction = static_cast<uint8_t>(dir);
    args->pull = static_cast<uint8_t>(pull);
    args->init_val = static_cast<uint8_t>(init_val);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

void Device::gpio_write(int idx, bool val) {
    iface_req_t req = {
        .periph_id   = PERIPH_GPIO_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = GPIO_WRITE_ID,
        .payload_len = sizeof(gpio_write_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<gpio_write_req_t*>(req.payload);
    args->val = static_cast<uint8_t>(val);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

bool Device::gpio_read(int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_GPIO_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = GPIO_READ_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != sizeof(gpio_read_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }
    auto ret = reinterpret_cast<gpio_read_resp_t*>(resp.payload);
    return ret->val;
}

};
