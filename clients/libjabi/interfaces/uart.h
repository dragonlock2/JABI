#ifndef LIBJABI_INTERFACES_UART_H
#define LIBJABI_INTERFACES_UART_H

#include "interface.h"

namespace jabi {

class UARTInterface : public Interface {
public:
    ~UARTInterface();

    iface_resp_t send_request(iface_req_t req);
    static Device get_device(std::string port, int baud);

private:
    UARTInterface(std::string port, int baud);

    int fd;
};

};

#endif // LIBJABI_INTERFACES_UART_H
