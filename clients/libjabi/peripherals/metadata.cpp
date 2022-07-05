#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/metadata.h>

std::string Device::serial() {
    iface_req_t req = {
        .periph_id   = PERIPH_METADATA_ID,
        .periph_idx  = 0,
        .periph_fn   = METADATA_SERIAL_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);

    resp.payload[std::min<size_t>(resp.payload_len, RESP_PAYLOAD_MAX_SIZE-1)] = 0x00;
    return std::string(reinterpret_cast<char*>(resp.payload));
}

int Device::num_inst(InstID id) {
    iface_req_t req = {
        .periph_id   = PERIPH_METADATA_ID,
        .periph_idx  = 0,
        .periph_fn   = METADATA_NUM_INST_ID,
        .payload_len = sizeof(metadata_num_inst_req_t),
        .payload     = {0},
    };

    auto args = reinterpret_cast<metadata_num_inst_req_t*>(req.payload);
    args->periph_id = htole<uint16_t>(static_cast<uint16_t>(id));

    iface_resp_t resp = interface->send_request(req);
    if (resp.payload_len != sizeof(metadata_num_inst_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = reinterpret_cast<metadata_num_inst_resp_t*>(resp.payload);
    return letoh<uint16_t>(ret->num_idx);
}

std::string Device::echo(std::string str) {
    if (str.length() > RESP_PAYLOAD_MAX_SIZE) {
        throw std::runtime_error("string too long");
    }

    iface_req_t req = {
        .periph_id   = PERIPH_METADATA_ID,
        .periph_idx  = 0,
        .periph_fn   = METADATA_ECHO_ID,
        .payload_len = static_cast<uint16_t>(str.length()),
        .payload     = {0},
    };
    memcpy(req.payload, str.c_str(), str.length());

    iface_resp_t resp = interface->send_request(req);
    return std::string(reinterpret_cast<char*>(resp.payload), resp.payload_len);
}

size_t Device::req_max_size() {
    iface_req_t req = {
        .periph_id   = PERIPH_METADATA_ID,
        .periph_idx  = 0,
        .periph_fn   = METADATA_REQ_MAX_SIZE_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);

    auto ret = reinterpret_cast<metadata_req_max_size_resp_t*>(resp.payload);
    return letoh<uint32_t>(ret->size);
}

size_t Device::resp_max_size() {
    iface_req_t req = {
        .periph_id   = PERIPH_METADATA_ID,
        .periph_idx  = 0,
        .periph_fn   = METADATA_RESP_MAX_SIZE_ID,
        .payload_len = 0,
        .payload     = {0},
    };

    iface_resp_t resp = interface->send_request(req);

    auto ret = reinterpret_cast<metadata_resp_max_size_resp_t*>(resp.payload);
    return letoh<uint32_t>(ret->size);
}

};
