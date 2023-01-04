use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerRead, DekuRead, DekuUpdate};

enum Func {
    Read = 0,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::ADC as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn adc_read(&self, idx: usize) -> Result<f32, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ReadResponse {
            mv: i32,
        }
        let resp = self.send(&gen_req(Func::Read, idx, vec![])?)?;
        match ReadResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret.mv as f32 * 0.001),
            _ => Err(Error::PacketFormat),
        }
    }
}
