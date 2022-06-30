#ifndef LIBJABI_DEVICE_H
#define LIBJABI_DEVICE_H

#include <iostream>
#include <memory>

namespace jabi {

#include <jabi/peripherals.h>

class Interface;

class Device {
public:
    /* Metadata */
    std::string serial();
    int num_inst(int periph_id);
    std::string echo(std::string str);

private:
    Device(std::shared_ptr<Interface> i) : interface(i) {}

    std::shared_ptr<Interface> interface;

    friend class Interface;
};

const int METADATA_ID = PERIPH_METADATA_ID;

};

#endif // LIBJABI_DEVICE_H
