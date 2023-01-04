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
        println!("\tListening only to 0x69420 messages on CAN {}", i);
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
            println!("\t{}", msg);
        }
    }
    println!();

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
    if test_device(&jabi::uart::get_device("/dev/tty.usbmodem14102", 230400).unwrap()).is_err() {
        println!("failed somewhere...");
    }
    println!("done!");
}
