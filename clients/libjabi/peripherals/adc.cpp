#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/adc.h>

int Device::adc_read(int idx) {
    iface_req_t req = {
        .periph_id   = PERIPH_ADC_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = ADC_READ_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != sizeof(adc_read_j_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }
    auto ret = reinterpret_cast<adc_read_j_resp_t*>(resp.payload);
    ret->mv = letoh<int32_t>(ret->mv);
    return ret->mv;
}

};
