fn test_device(d: &jabi::Device) -> Result<(), jabi::Error> {
    println!(
        "SN={} num_meta={:?} echo={} req_max_size={} resp_max_size={}",
        d.serial()?,
        d.num_inst(jabi::InstID::Metadata)?,
        String::from_utf8(d.echo("❤️".into())?).unwrap_or("🥹".into()), // uses Vec<u8>
        d.req_max_size()?,
        d.resp_max_size()?,
    );
    Ok(())
}

fn main() {
    test_device(&jabi::uart::get_device("/dev/tty.usbmodem14402", 230400).unwrap()).unwrap();
}
