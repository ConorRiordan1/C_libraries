import unittest
from constants import *
from helpers import *

class BadPacketTests(unittest.TestCase):
    # len and datalen mismatch for underflow/overflow
    def setUp(self):
        self.socket = set_up()
        self.admin_session = admin_login(self.socket)
  
    def test_3_data1_long(self):
        data1 = generate_alpha()
        data2 = generate_alpha()
        for k, v in OP_CODES.items():
            if k in DUAL_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, len(data1)+100, len(data2), data1+data2, 0)

            elif k in SINGLE_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, len(data1)+100, 0, data1, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_4_data2_long(self):
        # 1 char short
        data1 = generate_alpha()
        data2 = generate_alpha()
        for k, v in OP_CODES.items():
            if k in DUAL_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, len(data1), len(data2)+100, data1+data2, 0)
                self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_5_both_long(self):
        data1 = generate_alpha()
        data2 = generate_alpha()
        for k, v in OP_CODES.items():
            if k in DUAL_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, len(data1)+100, len(data2)+100, data1+data2, 0)

            elif k in SINGLE_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, len(data1)+100, 1, data1, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
    

    def test_7_bad_read(self):
        data1 = generate_alpha()
        data2 = generate_alpha()
        for k, v in OP_CODES.items():
            if k in DUAL_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, 65535, 65535, data1+data2, 0)

            elif k in SINGLE_ARG_OPS:
                packet = send_recv(self.socket, v, self.admin_session, 65535, 0, data1, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        self.socket.close()

class PartialSendRecvTests(unittest.TestCase):
    def setUp(self):
        self.socket = set_up()
        self.admin_session = admin_login(self.socket)
    
    def test_1_partial_send(self):
        total_bytes_sent = 0
        data = serialize(OP_CODES["register"], 0, 5, 4, "DaffyDuck", 0)
        bytes_to_send = len(data)

        while total_bytes_sent < bytes_to_send:
            # Determine the size of the next chunk
            chunk_size = min(len(data)//2, len(data)-total_bytes_sent)
            # Send the current chunk
            sent = self.socket.send(data[total_bytes_sent:total_bytes_sent + chunk_size])
            total_bytes_sent += sent
        self.socket.settimeout(1)
        try:
            packet = deserialize(self.socket.recv(BUF_SIZE), OP_CODES["register"])
        except TimeoutError:
            self.fail(msg="Didn't get a response from partial send/recv check")
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="Partial recv not implemented")
        self.socket.settimeout(None)

        packet = send_recv(self.socket, OP_CODES["login"], 0, len("Daffy"), len("Duck"), "DaffyDuck", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        self.socket.close()
    
class SuddenDisconnect(unittest.TestCase):

    def setUp(self):
        self.socket = set_up()
    
    def test_1_disconnect(self):
        data = serialize(OP_CODES["login"], 0, 5, 11, "RustyShackleford", 0)
        self.socket.send(data)
        self.socket.close()