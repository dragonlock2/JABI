#include <cmath>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/pwm.h>

void Device::pwm_write(int idx, double pulsewidth, double period) {
    iface_req_t req = {
        .periph_id   = PERIPH_PWM_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = PWM_WRITE_ID,
        .payload_len = sizeof(pwm_write_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<pwm_write_req_t*>(req.payload);
    args->pulsewidth = htole<uint32_t>(std::lround(pulsewidth * 1e9));
    args->period = htole<uint32_t>(std::lround(period * 1e9));
    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

};
