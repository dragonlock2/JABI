use crate::interfaces::{Interface, InterfaceRequest, InterfaceResponse};
use crate::{Device, Error};
use deku::{DekuContainerRead, DekuContainerWrite};
use std::sync::{Arc, Mutex};

const USB_TIMEOUT: std::time::Duration = std::time::Duration::from_millis(2000);

struct USBInterface<T: rusb::UsbContext> {
    dev: rusb::DeviceHandle<T>,
    ep_out_pkt_size: usize,
    ep_out: u8,
    ep_in: u8,
    max_req_size: usize,
    max_resp_size: usize,
}

impl<T: rusb::UsbContext> Interface for USBInterface<T> {
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
        match self.dev.write_bulk(self.ep_out, &breq, USB_TIMEOUT) {
            Ok(n) if n == breq.len() => (),
            _ => return Err(Error::PacketTimeout),
        }
        if breq.len() % self.ep_out_pkt_size == 0 // manually send ZLP
            && self.dev.write_bulk(self.ep_out, &[], USB_TIMEOUT).is_err()
        {
            return Err(Error::PacketTimeout);
        }

        // parse response
        let mut buf = vec![0; 4 + self.max_resp_size];
        match self.dev.read_bulk(self.ep_in, &mut buf, USB_TIMEOUT) {
            Ok(n) => match InterfaceResponse::from_bytes((&buf[0..n], 0)) {
                Ok(((s, _), resp)) if s.len() == 0 => match Error::from(resp.retcode) {
                    Error::NoError => Ok(resp.payload),
                    e => Err(e),
                },
                _ => Err(Error::PacketFormat),
            },
            Err(_) => Err(Error::PacketTimeout),
        }
    }

    fn set_max_req_size(&mut self, size: usize) {
        self.max_req_size = size;
    }

    fn set_max_resp_size(&mut self, size: usize) {
        self.max_resp_size = size;
    }

    fn reset(&mut self) {
        let _ = self.dev.reset();
    }
}

pub fn list_devices() -> Result<Vec<Device>, rusb::Error> {
    /* Driver will attach to an interface descriptor w/ following properties
     *   - bAlternateSetting of 0 (default)
     *   - bInterfaceClass of 0xFF (Vendor Specific)
     *   - iInterface string descriptor of "JABI USB"
     *   - only 2 bulk transfer endpoints (1 IN, 1 OUT)
     *   - responds to a req_max_size() request
     */
    let mut jabis = Vec::new();
    'device: for device in rusb::devices()?.iter() {
        if let Ok(cfg) = device.active_config_descriptor() {
            'interface: for interface in cfg.interfaces() {
                for desc in interface.descriptors() {
                    if desc.setting_number() != 0
                        || desc.class_code() != rusb::constants::LIBUSB_CLASS_VENDOR_SPEC
                        || desc.description_string_index().is_none()
                        || desc.num_endpoints() != 2
                    {
                        continue;
                    }
                    let ep0 = desc.endpoint_descriptors().nth(0).unwrap();
                    let ep1 = desc.endpoint_descriptors().nth(1).unwrap();
                    if ep0.transfer_type() != rusb::TransferType::Bulk
                        || ep1.transfer_type() != rusb::TransferType::Bulk
                        || ep0.direction() == ep1.direction()
                    {
                        continue;
                    }
                    if let Ok(mut dev) = device.open() {
                        if let Ok(s) = dev
                            .read_string_descriptor_ascii(desc.description_string_index().unwrap())
                        {
                            if s != "JABI USB" {
                                continue;
                            }
                        } else {
                            continue 'device;
                        }
                        if dev.claim_interface(interface.number()).is_err() {
                            continue 'interface;
                        }
                        let (ep_out, ep_in) = if ep0.direction() == rusb::Direction::Out {
                            (ep0, ep1)
                        } else {
                            (ep1, ep0)
                        };
                        let iface = USBInterface {
                            dev: dev,
                            ep_out_pkt_size: ep_out.max_packet_size() as usize,
                            ep_out: ep_out.address(),
                            ep_in: ep_in.address(),
                            max_req_size: u16::MAX as usize,
                            max_resp_size: u16::MAX as usize,
                        };
                        let jabi = Device {
                            iface: Arc::new(Mutex::new(iface)),
                        };
                        match jabi.req_max_size() {
                            Ok(req_size) => {
                                jabi.iface().set_max_req_size(req_size);
                                match jabi.resp_max_size() {
                                    Ok(resp_size) => jabi.iface().set_max_resp_size(resp_size),
                                    Err(_) => continue 'device,
                                }
                                jabis.push(jabi)
                            }
                            Err(_) => {
                                // Device auto-resets on packet timeout
                                match jabi.req_max_size() {
                                    Ok(req_size) => {
                                        jabi.iface().set_max_req_size(req_size);
                                        match jabi.resp_max_size() {
                                            Ok(resp_size) => {
                                                jabi.iface().set_max_resp_size(resp_size)
                                            }
                                            Err(_) => continue 'device,
                                        }
                                        jabis.push(jabi)
                                    }
                                    Err(_) => continue,
                                }
                            }
                        }
                    } else {
                        continue 'device;
                    }
                }
            }
        }
    }
    Ok(jabis)
}
