#ifndef JABI_H
#define JABI_H

#include <device.h>
#include <jabi/interfaces.h>
#include <jabi/peripherals.h>

#define JABI_IFACE_NODE  DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_interfaces)
#define JABI_PERIPH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_peripherals)

#define NUM_INTERFACES (DT_PROP(JABI_IFACE_NODE, usb) + \
                        DT_PROP_LEN(JABI_IFACE_NODE, uart))

extern void iface_req_to_le(iface_req_t *req);
extern void iface_resp_to_le(iface_resp_t *resp);

#endif // JABI_H
