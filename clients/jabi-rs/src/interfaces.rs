pub mod uart;
pub mod usb;

use crate::Error;
use deku::{DekuContainerWrite, DekuUpdate, DekuWrite};

pub const REQ_PAYLOAD_MAX_SIZE: usize = 128; // safe minimum

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

pub trait Interface {
    fn send(&mut self, req: &InterfaceRequest) -> Result<Vec<u8>, Error>;
    fn set_max_req_size(&mut self, size: usize);
}
