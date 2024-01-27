import jabi
import time

# see zephyrboards/boards/riscv/usb_pdmon/board.c

def set(d, mV, mA):
    d.custom(list(b'\x00' + mV.to_bytes(2, 'big') + mA.to_bytes(2, 'big')))

def get(d):
    raw = d.custom([1])
    mV = int.from_bytes(raw[0:2], 'big')
    mA = int.from_bytes(raw[2:4], 'big')
    return { "mV": mV, "mA": mA }

if __name__ == "__main__":
    d = jabi.USBInterface.list_devices()[0]
    set(d, 20000, 5000)
    time.sleep(1.0) # wait for request to happen
    print("actual: ", get(d))
    while True:
        mV = d.adc_read(1)
        mA = (d.adc_read(0) - 1500) * 1000 / 200 # ACS70331EESATR-005B3
        print(mV, "\bmV", mA, "\bmA")
        time.sleep(0.5)
