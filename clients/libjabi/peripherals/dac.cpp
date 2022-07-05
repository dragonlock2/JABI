#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/dac.h>

void Device::dac_write(int idx, int mV) {
    iface_req_t req = {
        .periph_id   = PERIPH_DAC_ID,
        .periph_idx  = static_cast<uint16_t>(idx),
        .periph_fn   = DAC_WRITE_ID,
        .payload_len = sizeof(dac_write_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<dac_write_req_t*>(req.payload);
    args->mv = htole<int32_t>(mV);
    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != 0) {
        throw std::runtime_error("unexpected payload length");
    }
}

};
