import jabi
import time

def testDevice(d):
    # Metadata
    print(
        "SN=" + d.serial(),
        "num_meta=" + str(d.num_inst(jabi.InstID.METADATA)),
        "echo=" + d.echo("❤️")
    )

    # CAN
    for i in range(d.num_inst(jabi.InstID.CAN)):
        print("\tListening only to 0x69420 messages on CAN", i)
        d.can_set_rate(125000, 1000000, i)
        d.can_set_filter(0x69420, 0xFFFFF, 0, 0, i)
        d.can_set_mode(jabi.CANMode.NORMAL, i)
        s = d.can_state(i)
        print("\tstate:", s.state, "tx_err:", s.tx_err, "rx_err:", s.rx_err)

        d.can_write(jabi.CANMessage(0x69420, [69, 42]), i)
        d.can_write(jabi.CANMessage(0x69420, 2), i)
        print("\tSent some messages")

        time.sleep(0.5)
        print("\tPrinting received messsages")
        while (msg := d.can_read(i)):
            print("\t", msg)
    print()

    # I2C
    for i in range(d.num_inst(jabi.InstID.I2C)):
        print("\tScanning for devices on I2C", i)
        d.i2c_set_freq(jabi.I2CFreq.STANDARD, i)
        for j in range(128):
            try:
                d.i2c_write(j, [], i)
                d.i2c_read(j, 0, i)
                print("\t Found", j)
            except:
                continue
    print()

if __name__ == "__main__":
    for d in jabi.USBInterface.list_devices():
        print("Found USB: ", end="")
        testDevice(d)

    print("Found UART: ", end="")
    testDevice(jabi.UARTInterface.get_device("COM5", 230400))
    print("done!")
