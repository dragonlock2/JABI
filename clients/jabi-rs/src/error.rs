#[derive(Debug)]
pub enum Error {
    Unknown,
    PacketTimeout,
    PacketFormat,
    NoError,
    NotSupported,
    InvalidArgsFormat,
    Uninitialized,
    Peripheral,
    InvalidArgs,
    Busy,
    Timeout,
}

impl From<i16> for Error {
    fn from(value: i16) -> Self {
        match value {
            0 => Self::NoError,
            1 => Self::NotSupported,
            2 => Self::InvalidArgsFormat,
            3 => Self::Uninitialized,
            4 => Self::Peripheral,
            5 => Self::InvalidArgs,
            6 => Self::Busy,
            7 => Self::Timeout,
            _ => Self::Unknown,
        }
    }
}
