use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

enum Func {
    Write = 0,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::DAC as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn dac_write(&self, idx: usize, volts: f32) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest {
            mv: i32,
        }
        let req = WriteRequest {
            mv: (volts * 1000.0) as i32,
        };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }
}
