use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

pub enum UARTParity {
    None = 0,
    Odd = 1,
    Even = 2,
    Mark = 3,
    Space = 4,
}

pub enum UARTStop {
    B0_5 = 0,
    B1 = 1,
    B1_5 = 2,
    B2 = 3,
}

enum Func {
    SetConfig = 0,
    Write = 1,
    Read = 2,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::UART as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn uart_set_config(
        &self,
        idx: usize,
        baud: u32,
        data_bits: u8,
        parity: UARTParity,
        stop: UARTStop,
    ) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetConfigRequest {
            baud: u32,
            data_bits: u8,
            parity: u8,
            stop_bits: u8,
        }
        let req = SetConfigRequest {
            baud,
            data_bits,
            parity: parity as u8,
            stop_bits: stop as u8,
        };
        let resp = self.send(&gen_req(Func::SetConfig, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn uart_write(&self, idx: usize, data: &[u8]) -> Result<(), Error> {
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

    pub fn uart_read(&self, idx: usize, len: usize) -> Result<Vec<u8>, Error> {
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
}
