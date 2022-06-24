#include <jabi/peripherals.h>
#include <jabi/peripherals/metadata.h>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

std::string Device::get_serial() {
    iface_req_t req = {
        .periph_id = PERIPH_METADATA_ID,
        .periph_idx = 0,
        .periph_fn = METADATA_GET_SERIAL_ID,
        .payload_len = 0,
    };

    iface_resp_t resp = interface->send_request(req);

    resp.payload[resp.payload_len] = 0x00; // null terminator
    return std::string((char*) resp.payload);
}

int Device::get_num_inst(int periph_id) {
    iface_req_t req = {
        .periph_id = PERIPH_METADATA_ID,
        .periph_idx = 0,
        .periph_fn = METADATA_GET_NUM_INST_ID,
        .payload_len = sizeof(metadata_get_num_inst_req_t),
    };

    auto args = reinterpret_cast<metadata_get_num_inst_req_t*>(req.payload);
    args->periph_id = htole<uint16_t>(periph_id);

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != sizeof(metadata_get_num_inst_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = reinterpret_cast<metadata_get_num_inst_resp_t*>(resp.payload);
    return letoh<uint16_t>(ret->num_idx);
}

};
