use std::time::Duration;

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

        std::thread::sleep(Duration::from_millis(500));
        println!("\tPrinting received messages");
        while let Some(msg) = d.can_read(i)? {
            println!("\t{msg}");
        }
    }
    println!();

    // LIN
    for i in 0..d.num_inst(jabi::InstID::LIN)? {
        println!("\tDoing some transactions as commander at 19.2kbps on LIN {i}");
        d.lin_set_mode(i, jabi::LINMode::Commander)?;
        d.lin_set_rate(i, 19200)?;
        for j in 0..64 {
            d.lin_set_filter(i, j, 0, jabi::LINChecksum::Auto)?;
        }
        d.lin_write(
            i,
            &jabi::LINMessage::new(42, vec![69, 42], jabi::LINChecksum::Enhanced),
        )?;
        println!("\t Sent a message");
        if let Ok(Some(msg)) = d.lin_read(i, 16) {
            println!("\t Received {}", msg);
        } else {
            println!("\t Didn't receive a message from 16");
        }

        println!("\tListening to messages as responder on LIN {i}");
        d.lin_set_mode(i, jabi::LINMode::Responder)?;
        d.lin_write(
            i,
            &jabi::LINMessage::new(16, vec![69, 42], jabi::LINChecksum::Enhanced),
        )?;
        println!("\t Queued a message on ID 16");
        std::thread::sleep(Duration::from_secs(1));
        let status = d.lin_status(i)?;
        if status.id == 16 && status.success {
            println!("\t Successfully sent message");
        } else {
            println!("\t Failed to send message");
        }
        println!("\t Printing received messages");
        while let Some(msg) = d.lin_read(i, 0xFF)? {
            println!("\t{msg}");
        }
    }
    println!();

    // SPI
    for i in 0..d.num_inst(jabi::InstID::SPI)? {
        println!("\tSetting SPI {i} to 250kHz, MODE0, LSB first");
        d.spi_set_freq(i, 250000)?;
        d.spi_set_mode(i, 0)?;
        d.spi_set_bitorder(i, false)?;

        d.spi_write(i, &vec![69])?;
        println!("\t Wrote 69");
        println!("\t Read {}", d.spi_read(i, 1)?[0]);
        println!(
            "\t Transceived out [1, 2, 3, 4], in {:?}",
            d.spi_transceive(i, &vec![1, 2, 3, 4])?
        );
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

    // UART
    for i in 0..d.num_inst(jabi::InstID::UART)? {
        d.uart_set_config(i, 115200, 8, jabi::UARTParity::None, jabi::UARTStop::B1)?;
        println!("\tSet UART {i} to 115200 baud, 8N1");
        d.uart_write(i, &vec![1, 2, 3, 4])?;
        println!("\t Sent [1, 2, 3, 4] to UART {i}");
        println!("\t Received {:?}", d.uart_read(i, 4)?);
    }
    println!();

    // PWM (GPIO overrides it until reset)
    for i in 0..d.num_inst(jabi::InstID::PWM)? {
        println!("\tFlashing PWM {i} at 1Hz");
        d.pwm_write(i, Duration::from_millis(500), Duration::from_millis(1000))?;
        std::thread::sleep(Duration::from_millis(100));
    }
    if d.num_inst(jabi::InstID::PWM)? > 0 {
        std::thread::sleep(Duration::from_secs(3));
    }

    // GPIO
    for i in 0..d.num_inst(jabi::InstID::GPIO)? {
        println!("\tFlashing GPIO {i}");
        d.gpio_set_mode(i, jabi::GPIODir::Output, jabi::GPIOPull::None, false)?;
        for _ in 0..6 {
            d.gpio_write(i, false)?;
            std::thread::sleep(Duration::from_millis(25));
            d.gpio_write(i, true)?;
            std::thread::sleep(Duration::from_millis(25));
        }
    }
    for i in 0..d.num_inst(jabi::InstID::GPIO)? {
        d.gpio_set_mode(i, jabi::GPIODir::Input, jabi::GPIOPull::Up, false)?;
        println!("\tRead GPIO {i} w/ pullups: {}", d.gpio_read(i)?);
    }
    println!();

    // ADC
    for i in 0..d.num_inst(jabi::InstID::ADC)? {
        println!("\tRead ADC {i} as {}V", d.adc_read(i)?);
    }
    println!();

    // DAC
    for i in 0..d.num_inst(jabi::InstID::DAC)? {
        println!("\tSetting DAC {i} to 690mV");
        d.dac_write(i, 0.690)?;
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
