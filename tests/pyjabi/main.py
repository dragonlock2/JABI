import jabi

def testDevice(d):
    print(
        "SN=" + d.get_serial(),
        "num_meta=" + str(d.get_num_inst(jabi.METADATA_ID)),
        "echo=" + d.echo("❤️")
    )

if __name__ == "__main__":
    for d in jabi.USBInterface.list_devices():
        print("Found USB: ", end="")
        testDevice(d)

    print("Found UART: ", end="")
    testDevice(jabi.UARTInterface.get_device("COM5", 230400))
    print("done!")
