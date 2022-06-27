#include <string>
#include <future>
#include <chrono>
#include "uart.h"

#ifndef _WIN32
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif // _WIN32

#define UART_TIMEOUT std::chrono::milliseconds(1000)

namespace jabi {

#ifdef _WIN32

UARTInterface::UARTInterface(std::string port, int baud) {
    hFile = CreateFileA(
        static_cast<LPCSTR>(("\\\\.\\" + port).c_str()),
        GENERIC_READ | GENERIC_WRITE,
        0, // exclusive access
        NULL, // no security
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("couldn't open COM port");
    }

    DCB dcb;
    if (!GetCommState(hFile, &dcb)) {
        throw std::runtime_error("couldn't get COM state");
    }

    // 8N1 and baud
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.BaudRate = baud;

    // no flow control
    dcb.fBinary       = true;
    dcb.fRtsControl   = RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow  = false;
    dcb.fDtrControl   = DTR_CONTROL_DISABLE;
    dcb.fOutxDsrFlow  = false;
    dcb.fOutX         = false;
    dcb.fInX          = false;
    dcb.fNull         = false;
    dcb.fErrorChar    = false;
    dcb.fAbortOnError = false;

    if (!SetCommState(hFile, &dcb)) {
        throw std::runtime_error("couldn't set COM state");
    }

    if (!PurgeComm(hFile, PURGE_RXABORT | PURGE_RXCLEAR |
                          PURGE_TXABORT | PURGE_TXCLEAR)) {
        throw std::runtime_error("couldn't purge COM port");
    }    
}

UARTInterface::~UARTInterface() {
    CloseHandle(hFile);
}

iface_resp_t UARTInterface::send_request(iface_req_t req) {
    std::future<iface_resp_t> lock = std::async([this, &req]{
        if (req.payload_len > REQ_PAYLOAD_MAX_SIZE) {
            throw std::runtime_error("request payload size too large");
        }
        iface_req_htole(req);
        DWORD len = IFACE_REQ_HDR_SIZE + req.payload_len;
        auto buffer = reinterpret_cast<char*>(&req);
        while (len) {
            DWORD sent_len;
            if (!WriteFile(hFile, buffer, len, &sent_len, NULL)) {
                throw std::runtime_error("write failed");
            }
            len -= sent_len;
            buffer += sent_len;
        }

        DWORD flags;
        COMSTAT comstat;
        if (!ClearCommError(hFile, &flags, &comstat)) {
            throw std::runtime_error("failed to clear error?");
        }

        iface_resp_t resp;
        resp.payload_len = 0;
        len = IFACE_RESP_HDR_SIZE;
        buffer = reinterpret_cast<char*>(&resp);
        while (len) {
            DWORD recv_len;
            if (!ReadFile(hFile, buffer, len, &recv_len, NULL)) {
                throw std::runtime_error("read failed");
            }
            len -= recv_len;
            buffer += recv_len;
        }
        iface_resp_letoh(resp);
        if (resp.retcode != 0 || resp.payload_len > REQ_PAYLOAD_MAX_SIZE) {
            throw std::runtime_error("bad response " + std::to_string(resp.retcode));
        }
        len = resp.payload_len;
        buffer = reinterpret_cast<char*>(resp.payload);
        while (len) {
            DWORD recv_len;
            if (!ReadFile(hFile, buffer, len, &recv_len, NULL)) {
                throw std::runtime_error("read failed");
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

#else

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

    if (tcsetattr(fd, TCSANOW, &tty)) {
        throw std::runtime_error("couldn't set TTY attributes");
    }

#ifdef __APPLE__
    #include <IOKit/serial/ioss.h>
    if (ioctl(fd, IOSSIOSPEED, &baud) == -1) {
        throw std::runtime_error("failed to set baud rate");
    }
#elif __linux__
    #include <asm/termios.h>
    struct termios2 tty2;
    if (ioctl(fd, TCGETS2, &tty2)) {
        throw std::runtime_error("failed to get termios2 config");
    }
    tty2.c_cflag &= ~CBAUD;
    tty2.c_cflag |= BOTHER;
    tty2.c_ispeed = baud;
    tty2.c_ospeed = baud;
    if (ioctl(fd, TCSETS2, &tty2)) {
        throw std::runtime_error("failed to set termios2 config");
    }
#else
    #error "OS not supported (yet?)"
#endif
    tcflush(fd, TCIOFLUSH);
}

UARTInterface::~UARTInterface() {
    close(fd);
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

        iface_resp_t resp;
        resp.payload_len = 0;
        len = IFACE_RESP_HDR_SIZE;
        buffer = reinterpret_cast<unsigned char*>(&resp);
        while (len) {
            int recv_len;
            if ((recv_len = read(fd, buffer, len)) < 0) {
                throw std::runtime_error("read failed");
            }
            len -= recv_len;
            buffer += recv_len;
        }
        iface_resp_letoh(resp);
        if (resp.retcode != 0 || resp.payload_len > REQ_PAYLOAD_MAX_SIZE) {
            throw std::runtime_error("bad response " + std::to_string(resp.retcode));
        }
        len = resp.payload_len;
        buffer = reinterpret_cast<unsigned char*>(resp.payload);
        while (len) {
            int recv_len;
            if ((recv_len = read(fd, buffer, len)) < 0) {
                throw std::runtime_error("read failed");
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

Device UARTInterface::get_device(std::string port, int baud) {
    return Interface::makeDevice(std::shared_ptr<UARTInterface>(
        new UARTInterface(port, baud)));
}

};
