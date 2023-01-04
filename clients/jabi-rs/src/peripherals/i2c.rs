use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

pub enum I2CFreq {
    Standard = 0, // 100kHz
    Fast = 1,     // 400kHz
    FastPlus = 2, // 1MHz
    High = 3,     // 3.4MHz
    Ultra = 4,    // 5MHz
}

enum Func {
    SetFreq = 0,
    Write = 1,
    Read = 2,
    Transceive = 3,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::I2C as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn i2c_set_freq(&self, idx: usize, preset: I2CFreq) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetFreqRequest {
            preset: u8,
        }
        let req = SetFreqRequest {
            preset: preset as u8,
        };
        let resp = self.send(&gen_req(Func::SetFreq, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn i2c_write(&self, idx: usize, addr: u16, data: &[u8]) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest<'a> {
            addr: u16,
            data: &'a [u8],
        }
        let req = WriteRequest { addr, data };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn i2c_read(&self, idx: usize, addr: u16, len: usize) -> Result<Vec<u8>, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct ReadRequest {
            addr: u16,
            data_len: u16,
        }
        let req = ReadRequest {
            addr,
            data_len: len as u16,
        };
        let resp = self.send(&gen_req(Func::Read, idx, req.to_bytes().unwrap())?)?;
        Ok(resp)
    }

    pub fn i2c_transceive(
        &self,
        idx: usize,
        addr: u16,
        data: &[u8],
        read_len: usize,
    ) -> Result<Vec<u8>, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct TransceiveRequest<'a> {
            addr: u16,
            data_len: u16,
            data: &'a [u8],
        }
        let req = TransceiveRequest {
            addr,
            data_len: read_len as u16,
            data,
        };
        let resp = self.send(&gen_req(Func::Transceive, idx, req.to_bytes().unwrap())?)?;
        Ok(resp)
    }
}
