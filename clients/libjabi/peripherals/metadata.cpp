#include <cstring>
#include <libjabi/byteorder.h>
#include <libjabi/interfaces/interface.h>

namespace jabi {

#include <jabi/peripherals.h>
#include <jabi/peripherals/metadata.h>

std::string Device::serial() {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_METADATA_ID,
            .periph_idx  = 0,
            .periph_fn   = METADATA_SERIAL_ID,
            .payload_len = 0,
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    return std::string(resp.payload.begin(), resp.payload.end());
}

int Device::num_inst(InstID id) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_METADATA_ID,
            .periph_idx  = 0,
            .periph_fn   = METADATA_NUM_INST_ID,
            .payload_len = sizeof(metadata_num_inst_req_t),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(sizeof(metadata_num_inst_req_t), 0),
    };

    auto args = reinterpret_cast<metadata_num_inst_req_t*>(req.payload.data());
    args->periph_id = htole<uint16_t>(static_cast<uint16_t>(id));

    iface_dynamic_resp_t resp = interface->send_request(req);
    if (resp.payload.size() != sizeof(metadata_num_inst_resp_t)) {
        throw std::runtime_error("unexpected payload length");
    }

    auto ret = reinterpret_cast<metadata_num_inst_resp_t*>(resp.payload.data());
    return letoh<uint16_t>(ret->num_idx);
}

std::string Device::echo(std::string str) {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_METADATA_ID,
            .periph_idx  = 0,
            .periph_fn   = METADATA_ECHO_ID,
            .payload_len = static_cast<uint16_t>(str.length()),
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(str.begin(), str.end()),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);
    return std::string(resp.payload.begin(), resp.payload.end());
}

size_t Device::req_max_size() {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_METADATA_ID,
            .periph_idx  = 0,
            .periph_fn   = METADATA_REQ_MAX_SIZE_ID,
            .payload_len = 0,
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);

    auto ret = reinterpret_cast<metadata_req_max_size_resp_t*>(resp.payload.data());
    return letoh<uint32_t>(ret->size);
}

size_t Device::resp_max_size() {
    iface_dynamic_req_t req = {
        .msg = {
            .periph_id   = PERIPH_METADATA_ID,
            .periph_idx  = 0,
            .periph_fn   = METADATA_RESP_MAX_SIZE_ID,
            .payload_len = 0,
            .payload     = {0},
        },
        .payload = std::vector<uint8_t>(),
    };

    iface_dynamic_resp_t resp = interface->send_request(req);

    auto ret = reinterpret_cast<metadata_resp_max_size_resp_t*>(resp.payload.data());
    return letoh<uint32_t>(ret->size);
}

};
