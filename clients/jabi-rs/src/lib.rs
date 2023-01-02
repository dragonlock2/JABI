mod device;
mod error;
mod interfaces;
mod peripherals {
    pub mod metadata;
}

pub use crate::device::Device;
pub use crate::error::Error;

pub use crate::interfaces::uart;
pub use crate::interfaces::usb;

pub use crate::peripherals::metadata::InstID;

// TODO re-export other enums, structs
