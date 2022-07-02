#ifndef LIBJABI_DEVICE_H
#define LIBJABI_DEVICE_H

#include <iostream>
#include <memory>
#include <vector>

namespace jabi {

#include <jabi/peripherals.h>

class Interface;

/* Metadata */
const int METADATA_ID = PERIPH_METADATA_ID;

/* CAN */
const int CAN_ID = PERIPH_CAN_ID;

enum class CANMode {
    NORMAL     = 0,
    LOOPBACK   = 1,
    LISTENONLY = 2,
};

struct CANState {
    int state;
    int tx_err;
    int rx_err;
};

struct CANMessage {
    int  id;
    bool ext;
    bool fd;
    bool brs;
    bool rtr;
    std::vector<uint8_t> data;

    CANMessage();
    CANMessage(int id, int req_len, bool fd=false, bool brs=false);
    CANMessage(int id, std::vector<uint8_t> data, bool fd=false, bool brs=false);
};

std::ostream &operator<<(std::ostream &os, CANMessage const &m);

class Device {
public:
    /* Metadata */
    std::string serial();
    int num_inst(int periph_id);
    std::string echo(std::string str);

    /* CAN */
    void can_set_filter(int id, int id_mask, bool rtr, bool rtr_mask, int idx=0);
    void can_set_rate(int bitrate, int bitrate_data, int idx=0);
    void can_set_mode(CANMode mode, int idx=0);
    CANState can_state(int idx=0);
    void can_write(CANMessage msg, int idx=0);
    int can_read(CANMessage &msg, int idx=0);

private:
    Device(std::shared_ptr<Interface> i) : interface(i) {}

    std::shared_ptr<Interface> interface;

    friend class Interface;
};

};

#endif // LIBJABI_DEVICE_H
