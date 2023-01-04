fn test_device(d: &jabi::Device) -> Result<(), jabi::Error> {
    // Metadata
    println!(
        "SN={} num_meta={} echo={} req_max_size={} resp_max_size={}",
        d.serial()?,
        d.num_inst(jabi::InstID::Metadata)?,
        String::from_utf8(d.echo("❤️".into())?).unwrap_or("🥹".into()), // uses Vec<u8>
        d.req_max_size()?,
        d.resp_max_size()?,
    );

    // CAN
    for i in 0..d.num_inst(jabi::InstID::CAN)? {
        println!("\tListening only to 0x69420 messages on CAN {i}");
        d.can_set_rate(i, 125000, 1000000)?;
        d.can_set_filter(i, 0x69420, 0xFFFFF, false, false)?;
        d.can_set_mode(i, jabi::CANMode::Normal)?;
        println!("\t{}", d.can_state(i)?);

        d.can_write(i, &jabi::CANMessage::new(0x69420, vec![69, 42], true))?;
        d.can_write(i, &jabi::CANMessage::new_rtr(0x69420, 2, true))?;
        println!("\tSent some messages");

        std::thread::sleep(std::time::Duration::from_millis(500));
        println!("\tPrinting received messages");
        while let Some(msg) = d.can_read(i)? {
            println!("\t{msg}");
        }
    }
    println!();

    // I2C
    for i in 0..d.num_inst(jabi::InstID::I2C)? {
        println!("\tScanning for devices on I2C {i}");
        d.i2c_set_freq(i, jabi::I2CFreq::Standard)?;
        for j in 0..128 {
            if d.i2c_write(i, j, &[]).is_ok() {
                println!("\t Found {j}");
            }
        }
    }
    println!();

    // GPIO
    for i in 0..d.num_inst(jabi::InstID::GPIO)? {
        println!("\tFlashing GPIO {i}");
        d.gpio_set_mode(i, jabi::GPIODir::Output, jabi::GPIOPull::None, false)?;
        for _ in 0..6 {
            d.gpio_write(i, false)?;
            std::thread::sleep(std::time::Duration::from_millis(25));
            d.gpio_write(i, true)?;
            std::thread::sleep(std::time::Duration::from_millis(25));
        }
    }
    for i in 0..d.num_inst(jabi::InstID::GPIO)? {
        d.gpio_set_mode(i, jabi::GPIODir::Input, jabi::GPIOPull::Up, false)?;
        println!("\tRead GPIO {i} w/ pullups: {}", d.gpio_read(i)?);
    }

    Ok(())
}

fn main() {
    for d in jabi::usb::list_devices().unwrap() {
        print!("Found USB: ");
        if test_device(&d).is_err() {
            println!("failed somewhere...");
        }
    }
    print!("Found UART: ");
    if test_device(&jabi::uart::get_device("COM5", 230400).unwrap()).is_err() {
        println!("failed somewhere...");
    }
    println!("done!");
}
