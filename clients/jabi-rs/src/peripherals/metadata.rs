use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerRead, DekuContainerWrite, DekuRead, DekuUpdate, DekuWrite};

pub enum InstID {
    Metadata = 0,
    CAN = 1,
}

enum Func {
    Serial = 0,
    NumInst = 1,
    Echo = 2,
    ReqMaxSize = 3,
    RespMaxSize = 4,
}

fn gen_req(f: Func, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: InstID::Metadata as u16,
        periph_idx: 0,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn serial(&self) -> Result<String, Error> {
        let resp = self.send(&gen_req(Func::Serial, vec![])?)?;
        match String::from_utf8(resp) {
            Ok(s) => Ok(s),
            Err(_) => Err(Error::Unknown),
        }
    }

    pub fn num_inst(&self, id: InstID) -> Result<usize, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct NumInstRequest {
            periph_id: u16,
        }
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct NumInstResponse {
            num_idx: u16,
        }
        let req = NumInstRequest {
            periph_id: id as u16,
        };
        let resp = self.send(&gen_req(Func::NumInst, req.to_bytes().unwrap())?)?;
        match NumInstResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret.num_idx as usize),
            _ => Err(Error::PacketFormat),
        }
    }

    pub fn echo(&self, s: Vec<u8>) -> Result<Vec<u8>, Error> {
        self.send(&gen_req(Func::Echo, s)?)
    }

    pub fn req_max_size(&self) -> Result<usize, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ReqMaxSizeResponse {
            size: u16,
        }
        let resp = self.send(&gen_req(Func::ReqMaxSize, vec![])?)?;
        match ReqMaxSizeResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret.size as usize),
            _ => Err(Error::PacketFormat),
        }
    }

    pub fn resp_max_size(&self) -> Result<usize, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct RespMaxSizeResponse {
            size: u16,
        }
        let resp = self.send(&gen_req(Func::RespMaxSize, vec![])?)?;
        match RespMaxSizeResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret.size as usize),
            _ => Err(Error::PacketFormat),
        }
    }
}
