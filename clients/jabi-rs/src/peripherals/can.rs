use crate::interfaces::InterfaceRequest;
use crate::Error;
use deku::{DekuContainerRead, DekuContainerWrite, DekuRead, DekuUpdate, DekuWrite};

pub enum CANMode {
    Normal = 0,
    Loopback = 1,
    ListenOnly = 2,
}

#[derive(DekuRead)] // happens to be the exact same format
#[deku(endian = "little")]
pub struct CANState {
    pub state: u8,
    pub tx_err: u8,
    pub rx_err: u8,
}

pub struct CANMessage {
    pub id: u32,
    pub ext: bool,
    pub fd: bool,
    pub brs: bool,
    pub rtr: bool,
    pub data: Vec<u8>,
}

impl std::fmt::Display for CANState {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "CANState(state={}, tx_err={}, rx_err={})",
            self.state, self.tx_err, self.rx_err
        )
    }
}

impl CANMessage {
    pub fn new(id: u32, data: Vec<u8>, fd_brs: bool) -> Self {
        Self {
            id: id,
            ext: id > 0x7FF,
            fd: fd_brs,
            brs: fd_brs,
            rtr: false,
            data: data,
        }
    }

    pub fn new_rtr(id: u32, len: usize, fd_brs: bool) -> Self {
        Self {
            id: id,
            ext: id > 0x7FF,
            fd: fd_brs,
            brs: fd_brs,
            rtr: true,
            data: vec![0; len],
        }
    }
}

impl std::fmt::Display for CANMessage {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "CANMessage(id={:#x}, ext={}, fd={}, brs={}, rtr={}, ",
            self.id, self.ext as u8, self.fd as u8, self.brs as u8, self.rtr as u8
        )?;
        if self.rtr {
            write!(f, "data.len()={}", self.data.len())?;
        } else {
            write!(f, "data={:?}", self.data)?;
        }
        write!(f, ")")
    }
}

const CAN_DATA_MAX_LEN: usize = 64;

enum Func {
    SetFilter = 0,
    SetRate = 1,
    SetMode = 2,
    State = 3,
    Write = 4,
    Read = 5,
}

fn gen_req(f: Func, idx: usize, payload: Vec<u8>) -> Result<InterfaceRequest, Error> {
    let mut r = InterfaceRequest {
        periph_id: crate::InstID::CAN as u16,
        periph_idx: idx as u16,
        periph_fn: f as u16,
        payload_len: 0,
        payload: payload,
    };
    r.update().map_or(Err(Error::PacketFormat), |_| Ok(r))
}

impl crate::Device {
    pub fn can_set_filter(
        &self,
        idx: usize, // Rust doesn't allow default args (yet)...
        id: u32,
        id_mask: u32,
    ) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetFilterRequest {
            id: u32,
            id_mask: u32,
        }
        let req = SetFilterRequest { id, id_mask };
        let resp = self.send(&gen_req(Func::SetFilter, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn can_set_rate(&self, idx: usize, bitrate: u32, bitrate_data: u32) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct SetRateRequest {
            bitrate: u32,
            bitrate_data: u32,
        }
        let req = SetRateRequest {
            bitrate,
            bitrate_data,
        };
        let resp = self.send(&gen_req(Func::SetRate, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn can_set_mode(&self, idx: usize, mode: CANMode) -> Result<(), Error> {
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

    pub fn can_state(&self, idx: usize) -> Result<CANState, Error> {
        let resp = self.send(&gen_req(Func::State, idx, vec![])?)?;
        match CANState::from_bytes((&resp, 0)) {
            Ok(((s, _), ret)) if s.len() == 0 => Ok(ret),
            _ => Err(Error::PacketFormat),
        }
    }

    pub fn can_write(&self, idx: usize, msg: &CANMessage) -> Result<(), Error> {
        #[derive(DekuWrite)]
        #[deku(endian = "little")]
        struct WriteRequest<'a> {
            id: u32,
            id_type: u8,
            fd: u8,
            brs: u8,
            rtr: u8,
            data_len: u8,
            data: &'a [u8],
        }
        if msg.data.len() > CAN_DATA_MAX_LEN {
            return Err(Error::InvalidArgs);
        }
        let req = WriteRequest {
            id: msg.id,
            id_type: msg.ext as u8,
            fd: msg.fd as u8,
            brs: msg.brs as u8,
            rtr: msg.rtr as u8,
            data_len: msg.data.len() as u8,
            data: if msg.rtr { &[] } else { &msg.data },
        };
        let resp = self.send(&gen_req(Func::Write, idx, req.to_bytes().unwrap())?)?;
        if resp.len() == 0 {
            Ok(())
        } else {
            Err(Error::PacketFormat)
        }
    }

    pub fn can_read(&self, idx: usize) -> Result<Option<CANMessage>, Error> {
        #[derive(DekuRead)]
        #[deku(endian = "little")]
        struct ReadResponse {
            _num_left: u16, // "simple" interface, remains unused
            id: u32,
            id_type: u8,
            fd: u8,
            brs: u8,
            rtr: u8,
            data_len: u8,
            // data[]
        }
        let resp = self.send(&gen_req(Func::Read, idx, vec![])?)?;
        if resp.len() == 0 {
            Ok(None)
        } else {
            match ReadResponse::from_bytes((&resp, 0)) {
                Ok(((s, _), ret))
                    if ret.rtr != 0 || (ret.rtr == 0 && s.len() == ret.data_len.into()) =>
                {
                    Ok(Some(CANMessage {
                        id: ret.id,
                        ext: ret.id_type != 0,
                        fd: ret.fd != 0,
                        brs: ret.brs != 0,
                        rtr: ret.rtr != 0,
                        data: if ret.rtr != 0 {
                            vec![0; ret.data_len.into()]
                        } else {
                            s.to_vec()
                        },
                    }))
                }
                _ => Err(Error::PacketFormat),
            }
        }
    }
}
