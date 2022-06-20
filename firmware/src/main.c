#include <zephyr.h>
#include <usb/usb_device.h>

int main() {
    usb_enable(NULL);

    int8_t idx = 0;
    while (1) {
        printk("Hello World! %d\r\n", idx++);

        k_msleep(1000);
    }

    return 0;
}
