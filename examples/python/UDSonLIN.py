import jabi
import time

class UDSonLIN:
    COMMANDER_ID = 0x3C
    RESPONDER_ID = 0x3D

    def __init__(self, dev, nad, rate=19200):
        dev.lin_set_mode(jabi.LINMode.COMMANDER)
        dev.lin_set_rate(rate)
        dev.lin_set_filter(self.RESPONDER_ID, 8, jabi.LINChecksum.CLASSIC)
        self.dev = dev
        self.nad = nad

    # low-level
    def _write_raw(self, data):
        assert(len(data) == 8)
        self.dev.lin_write(jabi.LINMessage(self.COMMANDER_ID, list(data), jabi.LINChecksum.CLASSIC))

    def _read_raw(self):
        msg = self.dev.lin_read(self.RESPONDER_ID)
        assert(msg.data[0] == self.nad)
        assert((msg.data[1] >> 4) <= 2)
        return bytes(msg.data)

    # transport layer
    def _write(self, data):
        assert(len(data) <= 4095)
        if len(data) <= 6:
            # SF
            self._write_raw(bytes([
                self.nad,
                0x00 | len(data),
                *data,
                *(b'\xff' * (6 - len(data)))
            ]))
        else:
            # FF
            self._write_raw(bytes([
                self.nad,
                0x10 | (len(data) // 256),
                len(data) % 256,
                *data[:5]
            ]))
            data = data[5:]
            # CF
            fc = 1
            while data:
                self._write_raw(bytes([
                    self.nad,
                    0x20 | fc,
                    *data[:6],
                    *(b'\xff' * (0 if len(data) >= 6 else (6 - len(data))))
                ]))
                fc = (fc + 1) % 16
                data = data[6:]

    def _read(self):
        pkt = self._read_raw()
        assert((pkt[1] >> 4) <= 1)
        if (pkt[1] >> 4) == 0:
            # SF
            length = pkt[1] & 0x0F
            assert(length <= 6)
            return pkt[2:2+length]
        else:
            # FF
            length = ((pkt[1] & 0x0F) << 8) | pkt[2]
            data = pkt[3:]
            length -= 5
            # CF
            fc = 1
            while length:
                pkt = self._read_raw()
                assert((pkt[1] >> 4) == 2)
                assert((pkt[1] & 0x0F) == fc)
                data += pkt[2:2+min(6,length)]
                fc = (fc + 1) % 16
                length -= min(6,length)
            return data

    # UDS layer
    # basic info - https://en.wikipedia.org/wiki/Unified_Diagnostic_Services
    # read/write DID - https://embetronicx.com/tutorials/automotive/uds-protocol/data-transmission-in-uds-protocol
    # routine format - https://embedclogic.com/uds-protocol/uds-remote-activation-of-routine/
    # routine subfunc - https://piembsystech.com/uds-protocol/
    def _routine(self, id, subfunc, data, delay):
        self._write(b'\x31' + subfunc + id.to_bytes(2, 'big') + data)
        time.sleep(delay)
        pkt = self._read()
        assert(len(pkt) >= 4)
        assert(pkt[0] == 0x71)
        assert(pkt[1] == subfunc[0])
        assert(int.from_bytes(pkt[2:4], 'big') == id)
        return pkt[4:]

    def routine_start(self, id, data, delay=0.0):
        return self._routine(id, b'\x01', data, delay)
    
    def routine_stop(self, id, data, delay=0.0):
        return self._routine(id, b'\x02', data, delay)
    
    def routine_request_results(self, id, data, delay=0.0):
        return self._routine(id, b'\x03', data, delay)

    def read_did(self, id, delay=0.0):
        self._write(b'\x22' + id.to_bytes(2, 'big'))
        time.sleep(delay)
        pkt = self._read()
        assert(len(pkt) >= 3)
        assert(pkt[0] == 0x62)
        assert(int.from_bytes(pkt[1:3], 'big') == id)
        return pkt[3:]
    
    def write_did(self, id, data, delay=0.0):
        self._write(b'\x2e' + id.to_bytes(2, 'big') + data)
        time.sleep(delay)
        pkt = self._read()
        assert(len(pkt) == 3)
        assert(pkt[0] == 0x6e)
        assert(int.from_bytes(pkt[1:3], 'big') == id)

"""
example usage:

uds = UDSonLIN(jabi.USBInterface.list_devices()[0], 0x00)
uds.read_did(0x0000)
"""
