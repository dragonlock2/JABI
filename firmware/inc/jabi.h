#ifndef JABI_H
#define JABI_H

#include <device.h>
#include <interfaces/common.h>

#define JABI_IFACE_NODE  DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_interfaces)
#define JABI_PERIPH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_peripherals)

#define NUM_INTERFACES (DT_PROP(JABI_IFACE_NODE, usb) + \
                        DT_PROP_LEN(JABI_IFACE_NODE, uart))

#endif // JABI_H