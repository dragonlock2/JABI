pub mod uart;
pub mod usb;

use crate::Error;
use deku::{DekuContainerWrite, DekuRead, DekuUpdate, DekuWrite};

#[derive(DekuWrite)]
#[deku(endian = "little")]
pub struct InterfaceRequest {
    pub periph_id: u16,
    pub periph_idx: u16,
    pub periph_fn: u16,

    #[deku(update = "self.payload.len()")]
    pub payload_len: u16,

    pub payload: Vec<u8>,
}

#[derive(DekuRead)]
#[deku(endian = "little")]
pub struct InterfaceResponse {
    pub retcode: i16,
    pub payload_len: u16,

    #[deku(count = "payload_len")]
    pub payload: Vec<u8>,
}

pub trait Interface {
    fn send(&mut self, req: &InterfaceRequest) -> Result<Vec<u8>, Error>;
    fn set_max_req_size(&mut self, size: usize);
    fn set_max_resp_size(&mut self, _size: usize) {}
    fn reset(&mut self) {}
}
