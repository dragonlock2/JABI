import jabi
import time

def testDevice(d):
    # Metadata
    print(
        "SN=" + d.serial(),
        "num_meta=" + str(d.num_inst(jabi.InstID.METADATA)),
        "echo=" + d.echo("❤️"),
        "req_max_size=" + str(d.req_max_size()),
        "resp_max_size=" + str(d.resp_max_size()),
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

    # LIN
    for i in range(d.num_inst(jabi.InstID.LIN)):
        print("\tDoing some transactions as commander at 19.2kbps on LIN", i)
        d.lin_set_mode(jabi.LINMode.COMMANDER, i)
        d.lin_set_rate(19200, i)
        for j in range(64):
            d.lin_set_filter(j, 0, jabi.LINChecksum.AUTO, i)
        d.lin_write(jabi.LINMessage(42, [69, 42], jabi.LINChecksum.ENHANCED), i)
        print("\t Sent a message")
        try:
            if msg := d.lin_read(16, i):
                print("\t Received", msg)
        except:
            print("\t Didn't receive a message from 16")

        print("\tListening to messages as responder on LIN", i)
        d.lin_set_mode(jabi.LINMode.RESPONDER, i)
        d.lin_write(jabi.LINMessage(16, [69, 42], jabi.LINChecksum.ENHANCED), i)
        print("\t Queued a message on ID 16")
        time.sleep(1)
        if (s := d.lin_status(idx=i)).id == 16 and s.success:
            print("\t Successfully sent message")
        else:
            print("\t Failed to send message")
        print("\t Printing received messages")
        while (msg := d.can_read(i)):
            print("\t", msg)

    # SPI
    for i in range(d.num_inst(jabi.InstID.SPI)):
        print("\tSetting SPI to 250kHz, MODE0, LSB first")
        d.spi_set_freq(250000, i)
        d.spi_set_mode(0, i)
        d.spi_set_bitorder(False, i)

        d.spi_write([69], i)
        print("\t Wrote 69")
        print("\t Read", d.spi_read(1, i)[0])
        print("\t Transceived out [1, 2, 3, 4], in ", d.spi_transceive([1,2,3,4], i))
    print()

    # I2C
    for i in range(d.num_inst(jabi.InstID.I2C)):
        print("\tScanning for devices on I2C", i)
        d.i2c_set_freq(jabi.I2CFreq.STANDARD, i)
        for j in range(128):
            try:
                d.i2c_write(j, [], i)
                # d.i2c_read(j, 0, i)
                print("\t Found", j)
            except:
                continue
    print()

    # UART
    for i in range(d.num_inst(jabi.InstID.UART)):
        try:
            d.uart_set_config(115200, 8, jabi.UARTParity.NONE, jabi.UARTStop.B1, i)
            print("\tSet UART", i, "to 115200 baud, 8N1")
            d.uart_write([1,2,3,4], i)
            print("\t Sent [1, 2, 3, 4] to UART", i)
            print("\t Received", d.uart_read(4, i))
        except:
            continue
    print()

    # PWM (GPIO overrides it until reset)
    for i in range(d.num_inst(jabi.InstID.PWM)):
        print("\tFlashing PWM", i, "at 1Hz")
        d.pwm_write(i, 0.5, 1.0)
        time.sleep(0.1)
    if d.num_inst(jabi.InstID.PWM) > 0:
        time.sleep(3)
    print()

    # GPIO
    for i in range(d.num_inst(jabi.InstID.GPIO)):
        print("\tFlashing GPIO", i)
        d.gpio_set_mode(i, jabi.GPIODir.OUTPUT)
        for _ in range(6):
            d.gpio_write(i, 0)
            time.sleep(0.025)
            d.gpio_write(i, 1)
            time.sleep(0.025)
    for i in range(d.num_inst(jabi.InstID.GPIO)):
        d.gpio_set_mode(i, jabi.GPIODir.INPUT, jabi.GPIOPull.UP)
        print("\tRead GPIO", i, "w/ pullups:", int(d.gpio_read(i)))
    print()

    # ADC
    for i in range(d.num_inst(jabi.InstID.ADC)):
        print("\tRead ADC", i, "as", d.adc_read(i), "\bmV")
    print()

    # DAC
    for i in range(d.num_inst(jabi.InstID.DAC)):
        print("\tSetting DAC", i, "to 690mV")
        d.dac_write(i, 690)
    print()

if __name__ == "__main__":
    for d in jabi.USBInterface.list_devices():
        print("Found USB: ", end="")
        testDevice(d)

    print("Found UART: ", end="")
    testDevice(jabi.UARTInterface.get_device("COM5", 230400))
    print("done!")
