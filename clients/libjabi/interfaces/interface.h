#ifndef LIBJABI_INTERFACES_INTERFACE_H
#define LIBJABI_INTERFACES_INTERFACE_H

#include <mutex>
#include <vector>
#include <libjabi/byteorder.h>
#include <libjabi/device.h>

namespace jabi {

#include <jabi/interfaces.h>

struct iface_dynamic_req_t {
    iface_req_t msg;
    std::vector<uint8_t> payload;
};

struct iface_dynamic_resp_t {
    iface_resp_t msg;
    std::vector<uint8_t> payload;
};

class Interface {
public:
    virtual ~Interface() = default;

    virtual iface_dynamic_resp_t send_request(iface_dynamic_req_t req) = 0;

    size_t get_req_max_size() { return req_max_size; }
    size_t get_resp_max_size() { return resp_max_size; }

protected:
    size_t req_max_size = REQ_PAYLOAD_MAX_SIZE;
    size_t resp_max_size = RESP_PAYLOAD_MAX_SIZE;
    std::mutex req_lock;

    static Device makeDevice(std::shared_ptr<Interface> i) { return Device(i); }
};

inline void iface_req_htole(iface_req_t &req) {
    req.periph_id   = htole<uint16_t>(req.periph_id);
    req.periph_idx  = htole<uint16_t>(req.periph_idx);
    req.periph_fn   = htole<uint16_t>(req.periph_fn);
    req.payload_len = htole<uint16_t>(req.payload_len);
}

inline void iface_resp_letoh(iface_resp_t &resp) {
    resp.retcode     = letoh<int16_t>(resp.retcode);
    resp.payload_len = letoh<uint16_t>(resp.payload_len);
}

};

#endif // LIBJABI_INTERFACES_INTERFACE_H
