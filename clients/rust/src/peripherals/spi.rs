use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

enum Func {
    SetFreq = 0,
    SetMode = 1,
    SetBitorder = 2,
    Write = 3,
    Read = 4,
    Transceive = 5,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::SPI as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn spi_set_freq(&self, idx: usize, freq: u32) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetFreqRequest {
            freq: u32,
        }
        let req = SetFreqRequest { freq };
        let resp = self.send(&gen_req(Func::SetFreq, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn spi_set_mode(&self, idx: usize, mode: u8) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetModeRequest {
            mode: u8,
        }
        let req = SetModeRequest { mode };
        let resp = self.send(&gen_req(Func::SetMode, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn spi_set_bitorder(&self, idx: usize, msb: bool) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetBitorderRequest {
            order: u8,
        }
        let req = SetBitorderRequest { order: msb as u8 };
        let resp = self.send(&gen_req(Func::SetBitorder, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn spi_write(&self, idx: usize, data: &[u8]) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest<'a> {
            data: &'a [u8],
        }
        let req = WriteRequest { data };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn spi_read(&self, idx: usize, len: usize) -> Result<Vec<u8>, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct ReadRequest {
            data_len: u16,
        }
        let req = ReadRequest {
            data_len: len as u16,
        };
        let resp = self.send(&gen_req(Func::Read, idx, req.to_bytes().unwrap())?)?;
        Ok(resp)
    }

    pub fn spi_transceive(&self, idx: usize, data: &[u8]) -> Result<Vec<u8>, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct TransceiveRequest<'a> {
            data: &'a [u8],
        }
        let req = TransceiveRequest { data };
        let resp = self.send(&gen_req(Func::Transceive, idx, req.to_bytes().unwrap())?)?;
        Ok(resp)
    }
}
