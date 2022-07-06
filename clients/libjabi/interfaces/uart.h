#ifndef LIBJABI_INTERFACES_UART_H
#define LIBJABI_INTERFACES_UART_H

#include "interface.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

namespace jabi {

class UARTInterface : public Interface {
public:
    ~UARTInterface();

    iface_dynamic_resp_t send_request(iface_dynamic_req_t req);
    static Device get_device(std::string port, int baud);

private:
    UARTInterface(std::string port, int baud);

#ifdef _WIN32
    HANDLE hFile;
#else
    int fd;
#endif // _WIN32
};

};

#endif // LIBJABI_INTERFACES_UART_H
