#ifndef JABI_H
#define JABI_H

#include <device.h>
#include <jabi/error.h>
#include <jabi/interfaces.h>
#include <jabi/peripherals.h>

#define JABI_IFACE_NODE  DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_interfaces)
#define JABI_PERIPH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_peripherals)
#define JABI_ADC_NODE    DT_COMPAT_GET_ANY_STATUS_OKAY(jabi_adc)

#define NUM_INTERFACES (DT_PROP(JABI_IFACE_NODE, usb) + \
                        DT_PROP_LEN(JABI_IFACE_NODE, uart))

extern void iface_req_to_le(iface_req_t *req);
extern void iface_resp_to_le(iface_resp_t *resp);

#define ELEM_TO_DEVICE(node_id, prop, idx) \
    DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)),

#define ELEM_TO_GPIO(node_id, prop, idx) \
    GPIO_DT_SPEC_GET(DT_PROP_BY_IDX(node_id, prop, idx), gpios),

#define ELEM_TO_PWM(node_id, prop, idx) \
    PWM_DT_SPEC_GET(DT_PROP_BY_IDX(node_id, prop, idx)),

#endif // JABI_H
