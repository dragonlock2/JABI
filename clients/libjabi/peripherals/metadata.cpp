#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/metadata.h>

std::string Device::get_serial() {
    iface_req_t req = {
        .periph_id = PERIPH_METADATA_ID,
        .periph_idx = 0,
        .periph_fn = METADATA_GET_SERIAL_ID,
        .payload_len = 0,
    };

    iface_resp_t resp = interface->send_request(req);

    resp.payload[std::min<size_t>(resp.payload_len, RESP_PAYLOAD_MAX_SIZE-1)] = 0x00;
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

std::string Device::echo(std::string str) {
    if (str.length() > RESP_PAYLOAD_MAX_SIZE-1) {
        throw std::runtime_error("string too long");
    }

    iface_req_t req = {
        .periph_id = PERIPH_METADATA_ID,
        .periph_idx = 0,
        .periph_fn = METADATA_ECHO_ID,
        .payload_len = static_cast<uint16_t>(str.length()+1), // include null
    };
    std::memcpy(req.payload, str.c_str(), req.payload_len);

    iface_resp_t resp = interface->send_request(req);

    resp.payload[RESP_PAYLOAD_MAX_SIZE-1] = 0x00; // just in case
    return std::string((char*) resp.payload);
}

};
