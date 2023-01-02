// TODO add concurrency support, clone helper

use crate::interfaces::{Interface, InterfaceRequest};
use crate::Error;
use std::sync::{Arc, Mutex};

pub struct Device {
    pub(crate) iface: Arc<Mutex<dyn Interface>>,
}

impl Device {
    pub(crate) fn send(&self, req: &InterfaceRequest) -> Result<Vec<u8>, Error> {
        self.iface.lock().unwrap().send(req)
    }
}
