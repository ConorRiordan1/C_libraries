import unittest
from helpers import *
import threading
""" TODO:
 Create many many users
 Create many many links
 create many many locations
 create many many admins
 create many many connections
 overly large packets
"""


class MassUsers(unittest.TestCase):
    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        print("here")

    def test_1(self):
        idx = 0
        while(idx != 1):
            thing = set_up()
            idx = idx + 1
        just_send(thing, OP_CODES["register"], 0, 0,0,"apple", 0)
        self.assertEqual(1,1)


    def test_2(self):

        def create_client(i):
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect(('localhost', 8000))  # replace with your server's IP and port
                print(f"Client {i} connected")
                # Add your logic here to interact with the server
                s.close()
            except socket.error as e:
                print(f"Client {i} had an error: {e}")

        num_clients = 300  # replace with the number of clients you want to simulate

        for i in range(num_clients):
            threading.Thread(target=create_client, args=(i,)).start()


class MassLinks(unittest.TestCase):
    pass

class MassLocations(unittest.TestCase):
    pass

class MassAdmins(unittest.TestCase):
    pass

class LargePackets(unittest.TestCase):
    pass