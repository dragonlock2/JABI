#ifndef LIBJABI_INTERFACES_UART_H
#define LIBJABI_INTERFACES_UART_H

#include "interface.h"

namespace jabi {

class UARTInterface : public Interface {
public:
    iface_resp_t send_request(iface_req_t req);
};

};

#endif // LIBJABI_INTERFACES_UART_H
