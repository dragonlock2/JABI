#ifndef LIBJABI_INTERFACES_INTERFACE_H
#define LIBJABI_INTERFACES_INTERFACE_H

#include <vector>
#include <libjabi/byteorder.h>
#include <libjabi/device.h>

namespace jabi {

#include <jabi/interfaces.h>

class Interface {
public:
    virtual ~Interface() = default;

    virtual iface_resp_t send_request(iface_req_t req) = 0;

protected:
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
