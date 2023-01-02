// TODO add concurrency support, clone helper

use crate::interfaces::{Interface, InterfaceRequest};
use crate::Error;
use std::sync::{Arc, Mutex, MutexGuard};

pub struct Device {
    pub(crate) iface: Arc<Mutex<dyn Interface>>,
}

impl Device {
    pub(crate) fn iface(&self) -> MutexGuard<'_, (dyn Interface + 'static)> {
        self.iface.lock().unwrap()
    }

    pub(crate) fn send(&self, req: &InterfaceRequest) -> Result<Vec<u8>, Error> {
        let mut iface = self.iface();
        match iface.send(req) {
            Err(Error::PacketTimeout) => {
                iface.reset();
                Err(Error::PacketTimeout)
            }
            v => v,
        }
    }
}
