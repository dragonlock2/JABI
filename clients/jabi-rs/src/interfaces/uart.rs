use crate::interfaces::{Interface, InterfaceRequest};
use crate::{Device, Error};
use deku::DekuContainerWrite;
use serial::SerialPort;
use std::sync::{Arc, Mutex};

const UART_TIMEOUT: std::time::Duration = std::time::Duration::from_millis(2000);

struct UARTInterface<T: SerialPort> {
    port: T,
    max_req_size: usize,
}

impl<T: SerialPort> Interface for UARTInterface<T> {
    fn send(&mut self, req: &InterfaceRequest) -> Result<Vec<u8>, Error> {
        // send request
        if req.payload.len() > self.max_req_size {
            // fail faster, not actually needed
            return Err(Error::PacketFormat);
        }
        let breq = match req.to_bytes() {
            Ok(v) => v,
            Err(_) => return Err(Error::PacketFormat),
        };
        if self.port.write_all(&breq).is_err() {
            return Err(Error::PacketTimeout);
        }

        // manually parse response
        let mut hdr = [0; 4];
        if self.port.read_exact(&mut hdr).is_err() {
            return Err(Error::PacketTimeout);
        }
        match Error::from(i16::from_le_bytes(hdr[0..2].try_into().unwrap())) {
            Error::NoError => (),
            e => return Err(e),
        }
        let mut payload = vec![0; u16::from_le_bytes(hdr[2..4].try_into().unwrap()).into()]; // max at 64KB, no check
        if self.port.read_exact(&mut payload).is_err() {
            return Err(Error::PacketTimeout);
        }
        Ok(payload)
    }

    fn set_max_req_size(&mut self, size: usize) {
        self.max_req_size = size;
    }
}

pub fn get_device(port: &str, baud: usize) -> Result<Device, serial::Error> {
    // configure port
    let mut iface = UARTInterface {
        port: serial::open(port)?,
        max_req_size: u16::MAX as usize,
    };
    iface.port.configure(&serial::PortSettings {
        baud_rate: serial::BaudRate::BaudOther(baud),
        char_size: serial::CharSize::Bits8,
        parity: serial::Parity::ParityNone,
        stop_bits: serial::StopBits::Stop1,
        flow_control: serial::FlowControl::FlowNone,
    })?;
    iface.port.set_timeout(UART_TIMEOUT)?;

    // check device responds
    let dev = Device {
        iface: Arc::new(Mutex::new(iface)),
    };
    match dev.req_max_size() {
        Ok(s) => {
            dev.iface().set_max_req_size(s);
            Ok(dev)
        }
        Err(_) => Err(serial::Error::new(
            serial::ErrorKind::NoDevice,
            "bad device response",
        )),
    }
}
