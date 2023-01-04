use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerRead, DekuContainerWrite, DekuRead, DekuUpdate, DekuWrite};

pub enum LINMode {
    Commander = 0,
    Responder = 1,
}

#[derive(Debug, Copy, Clone)]
pub enum LINChecksum {
    Classic = 0,
    Enhanced = 1,
    Auto = 2,
}

pub struct LINStatus {
    pub id: u8,
    pub success: bool,
}

pub struct LINMessage {
    pub id: u8,
    pub checksum_type: LINChecksum,
    pub data: Vec<u8>,
}

impl std::fmt::Display for LINStatus {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "LINStatus(id={}, success={})", self.id, self.success)
    }
}

impl LINMessage {
    pub fn new(id: u8, data: Vec<u8>, checksum_type: LINChecksum) -> Self {
        Self {
            id,
            checksum_type,
            data,
        }
    }
}

impl std::fmt::Display for LINMessage {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "LINMessage(id={:#x}, type={:?}, data={:?})",
            self.id, self.checksum_type, self.data
        )
    }
}

const LIN_DATA_MAX_LEN: usize = 8;

enum Func {
    SetMode = 0,
    SetRate = 1,
    SetFilter = 2,
    Mode = 3,
    Status = 4,
    Write = 5,
    Read = 6,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::LIN as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn lin_set_mode(&self, idx: usize, mode: LINMode) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetModeRequest {
            mode: u8,
        }
        let req = SetModeRequest { mode: mode as u8 };
        let resp = self.send(&gen_req(Func::SetMode, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn lin_set_rate(&self, idx: usize, bitrate: u32) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetRateRequest {
            bitrate: u32,
        }
        let req = SetRateRequest { bitrate };
        let resp = self.send(&gen_req(Func::SetRate, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn lin_set_filter(
        &self,
        idx: usize,
        id: u8,
        len: usize,
        checksum_type: LINChecksum,
    ) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetFilterRequest {
            id: u8,
            checksum_type: u8,
            data_len: u8,
        }
        if len > LIN_DATA_MAX_LEN {
            return Err(Error::InvalidArgs);
        }
        let req = SetFilterRequest {
            id,
            checksum_type: checksum_type as u8,
            data_len: len as u8,
        };
        let resp = self.send(&gen_req(Func::SetFilter, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn lin_mode(&self, idx: usize) -> Result<LINMode, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ModeResponse {
            id: u8,
        }
        let resp = self.send(&gen_req(Func::Mode, idx, vec![])?)?;
        match ModeResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(match ret.id {
                0 => LINMode::Commander,
                1 => LINMode::Responder,
                _ => return Err(Error::PacketFormat),
            }),
            _ => Err(Error::PacketFormat),
        }
    }

    pub fn lin_status(&self, idx: usize) -> Result<LINStatus, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct StatusResponse {
            id: u8,
            retcode: i16,
        }
        let resp = self.send(&gen_req(Func::Status, idx, vec![])?)?;
        match StatusResponse::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(LINStatus {
                id: ret.id,
                success: ret.retcode == 0,
            }),
            _ => Err(Error::PacketFormat),
        }
    }

    pub fn lin_write(&self, idx: usize, msg: &LINMessage) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest<'a> {
            id: u8,
            checksum_type: u8,
            data: &'a [u8],
        }
        if msg.data.len() > LIN_DATA_MAX_LEN {
            return Err(Error::InvalidArgs);
        }
        let req = WriteRequest {
            id: msg.id,
            checksum_type: msg.checksum_type as u8,
            data: &msg.data,
        };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn lin_read(&self, idx: usize, id: u8) -> Result<Option<LINMessage>, Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct ReadRequest {
            id: u8,
        }
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ReadResponse {
            _num_left: u16, // "simple" interface, remains unused
            id: u8,
            checksum_type: u8,
            // data[]
        }
        let req = ReadRequest { id };
        let resp = self.send(&gen_req(Func::Read, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(None)
        } else {
            match ReadResponse::from_bytes((&resp, 0)) {
                Ok(((s, _), ret)) => Ok(Some(LINMessage {
                    id: ret.id,
                    checksum_type: match ret.checksum_type {
                        0 => LINChecksum::Classic,
                        1 => LINChecksum::Enhanced,
                        _ => return Err(Error::PacketFormat),
                    },
                    data: s.to_vec(),
                })),
                _ => Err(Error::PacketFormat),
            }
        }
    }
}
