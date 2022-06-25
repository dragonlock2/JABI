#include "uart.h"

#ifdef _WIN32
#error "Windows not supported (yet)!"
#else
#include <future>
#include <chrono>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif // _WIN32

namespace jabi {

#ifdef _WIN32
#error "Windows not supported (yet)!"
#else

#define UART_TIMEOUT std::chrono::milliseconds(1000)

UARTInterface::UARTInterface(std::string port, int baud) {
    if ((fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {
        throw std::runtime_error("couldn't open port");
    }

    struct termios tty;
    if (tcgetattr(fd, &tty)) {
        throw std::runtime_error("couldn't get TTY attributes");
    }

    cfmakeraw(&tty); // set to raw mode
    tty.c_cflag &= ~CSIZE; // 8 bits
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB; // no parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit

    tty.c_cflag &= ~CRTSCTS; // no flow control
    tty.c_cflag |= CLOCAL;
    tty.c_cflag |= CREAD; // turn on read

    tty.c_cc[VMIN]  = 0; // return even if 0 bytes received
    tty.c_cc[VTIME] = 0; // zero read timeout

    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty)) {
        throw std::runtime_error("couldn't set TTY attributes");
    }

#ifdef __APPLE__
    #include <IOKit/serial/ioss.h>
    if (ioctl(fd, IOSSIOSPEED, &baud) == -1) {
        throw std::runtime_error("failed to set baud rate");
    }
#elif __linux__
    #error "Linux not supported (yet)!"
#else
    #error "OS not supported (yet?)"
#endif // __APPLE__

    (void)baud;
}

UARTInterface::~UARTInterface() {
    close(fd);
}

Device UARTInterface::get_device(std::string port, int baud) {
    return Interface::makeDevice(std::shared_ptr<UARTInterface>(
        new UARTInterface(port, baud)));
}

iface_resp_t UARTInterface::send_request(iface_req_t req) {
    std::future<iface_resp_t> lock = std::async([this, &req]{
        if (req.payload_len > REQ_PAYLOAD_MAX_SIZE) {
            throw std::runtime_error("request payload size too large");
        }
        iface_req_htole(req);
        int len = IFACE_REQ_HDR_SIZE + req.payload_len;
        auto buffer = reinterpret_cast<unsigned char*>(&req);
        while (len) {
            int sent_len;
            if ((sent_len = write(fd, buffer, len)) < 0) {
                throw std::runtime_error("write failed");
            }
            len -= sent_len;
            buffer += sent_len;
        }

        iface_resp_t resp = { .payload_len = 0 };
        len = IFACE_RESP_HDR_SIZE;
        buffer = reinterpret_cast<unsigned char*>(&resp);
        while (len) {
            int recv_len;
            if ((recv_len = read(fd, buffer, len)) < 0) {
                throw std::runtime_error("write failed");
            }
            len -= recv_len;
            buffer += recv_len;
        }
        iface_resp_letoh(resp);
        if (resp.retcode != 0 || resp.payload_len > REQ_PAYLOAD_MAX_SIZE) {
            throw std::runtime_error("bad response");
        }
        len = resp.payload_len;
        buffer = reinterpret_cast<unsigned char*>(resp.payload);
        while (len) {
            int recv_len;
            if ((recv_len = read(fd, buffer, len)) < 0) {
                throw std::runtime_error("write failed");
            }
            len -= recv_len;
            buffer += recv_len;
        }

        return resp;
    });

    if (lock.wait_for(UART_TIMEOUT) == std::future_status::timeout) {
        throw std::runtime_error("UART timeout");
    }
    return lock.get();
}

#endif // _WIN32

};
