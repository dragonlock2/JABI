#ifndef LIBJABI_INTERFACES_INTERFACE_H
#define LIBJABI_INTERFACES_INTERFACE_H

#include <vector>
#include <jabi/interfaces.h>
#include <libjabi/device.h>

namespace jabi {

class Interface {
public:
    virtual ~Interface() = default;

    virtual iface_resp_t send_request(iface_req_t req) = 0;

protected:
    static Device makeDevice(std::shared_ptr<Interface> i) { return Device(i); }
};

};

#endif // LIBJABI_INTERFACES_INTERFACE_H
