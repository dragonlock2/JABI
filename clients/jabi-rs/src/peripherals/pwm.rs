use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

enum Func {
    Write = 0,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::PWM as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn pwm_write(
        &self,
        idx: usize,
        pulsewidth: std::time::Duration,
        period: std::time::Duration,
    ) -> Result<(), Error> {
        if pulsewidth.as_nanos() > u32::MAX.into() || period.as_nanos() > u32::MAX.into() {
            return Err(Error::InvalidArgs);
        }
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest {
            pulsewidth: u32, // ns
            period: u32,
        }
        let req = WriteRequest {
            pulsewidth: pulsewidth.as_nanos() as u32,
            period: period.as_nanos() as u32,
        };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }
}
