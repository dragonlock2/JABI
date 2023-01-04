mod device;
mod error;
mod interfaces;
mod peripherals {
    pub mod adc;
    pub mod can;
    pub mod dac;
    pub mod gpio;
    pub mod i2c;
    pub mod lin;
    pub mod metadata;
    pub mod pwm;
    pub mod spi;
    pub mod uart;
}

pub use crate::device::Device;
pub use crate::error::Error;

pub use crate::interfaces::uart;
pub use crate::interfaces::usb;

pub use crate::peripherals::can::CANMessage;
pub use crate::peripherals::can::CANMode;
pub use crate::peripherals::can::CANState;
pub use crate::peripherals::gpio::GPIODir;
pub use crate::peripherals::gpio::GPIOPull;
pub use crate::peripherals::i2c::I2CFreq;
pub use crate::peripherals::lin::LINChecksum;
pub use crate::peripherals::lin::LINMessage;
pub use crate::peripherals::lin::LINMode;
pub use crate::peripherals::lin::LINStatus;
pub use crate::peripherals::metadata::InstID;
pub use crate::peripherals::uart::UARTParity;
pub use crate::peripherals::uart::UARTStop;
