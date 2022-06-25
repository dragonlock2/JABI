#ifndef LIBJABI_DEVICE_H
#define LIBJABI_DEVICE_H

#include <iostream>

namespace jabi {

class Interface;

class Device {
public:
    /* Metadata */
    std::string get_serial();
    int get_num_inst(int periph_id);
    std::string echo(std::string str);

private:
    Device(std::shared_ptr<Interface> i) : interface(i) {}

    std::shared_ptr<Interface> interface;

    friend class Interface;
};

};

#endif // LIBJABI_DEVICE_H
