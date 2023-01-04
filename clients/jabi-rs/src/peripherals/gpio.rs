use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerRead, DekuContainerWrite, DekuRead, DekuUpdate, DekuWrite};

pub enum GPIODir {
    Input = 0,
    Output = 1,
    OpenDrain = 2,
    OpenSource = 3,
}

pub enum GPIOPull {
    None = 0,
    Up = 1,
    Down = 2,
    Both = 3,
}

enum Func {
    SetMode = 0,
    Write = 1,
    Read = 2,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::GPIO as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn gpio_set_mode(
        &self,
        idx: usize,
        dir: GPIODir,
        pull: GPIOPull,
        init_val: bool,
    ) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetModeRequest {
            direction: u8,
            pull: u8,
            init_val: u8,
        }
        let req = SetModeRequest {
            direction: dir as u8,
            pull: pull as u8,
            init_val: init_val as u8,
        };
        let resp = self.send(&gen_req(Func::SetMode, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn gpio_write(&self, idx: usize, val: bool) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest {
            val: u8,
        }
        let req = WriteRequest { val: val as u8 };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn gpio_read(&self, idx: usize) -> Result<bool, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ReadResponse {
            val: u8,
        }
        let resp = self.send(&gen_req(Func::Read, idx, vec![])?)?;
        match ReadResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret.val != 0),
            _ => Err(Error::PacketFormat),
        }
    }
}
