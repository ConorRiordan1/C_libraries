import unittest
from helpers import *
from constants import *
from stars_test import STARMAP
from math import inf 

""" TODO:
 many actions at once
"""


class RaceLocations(unittest.TestCase):
    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.sockets = []
        self.session_ids = []
        for _ in range(21):
            self.sockets.append(set_up())
    
        self.session_ids.append(admin_login(self.sockets[0]))
        count = 1
        # create every new admin and then log them in on seperate file descriptors
        for k,v in CREDENTIALS.items():
            if count > 20:
                break
            send_recv(self.sockets[0], OP_CODES["admin_add"], self.session_ids[0], len(k), len(v), k+v, 0)
            packet = send_recv(self.sockets[count], OP_CODES["login"], self.session_ids[0], len(k), len(v), k+v, 0)
            self.session_ids.append(packet.session_id)
            count += 1

    def test_1_location_add_del_good(self):
        """Test what happens if 20 file descriptors all send requests to add/del different locations at once
        """
        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["location_add"], self.session_ids[index], index+1, 0, "A"*(index+1), 0)
            conn.send(data)

        for index, conn in enumerate(self.sockets):
            packet = deserialize(conn.recv(BUF_SIZE), OP_CODES["location_add"])
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Unable to add location: {'A'*(index+1)}")

        # repeat for deletion
        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["location_del"], self.session_ids[index], index+1, 0, "A"*(index+1), 0)
            conn.send(data)

        for index, conn in enumerate(self.sockets):
            packet = deserialize(conn.recv(BUF_SIZE), OP_CODES["location_del"])
            self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        
    def test_2_location_add_del_bad(self):
        """Test what happens if 20 file descriptors all send requests to add/del SAME locations at once
        """
        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["location_add"], self.session_ids[index], 8, 0, "Pylandia", 0)
            conn.send(data)

        # nothing to really check here because you can't really know if it succeeded or not, must run with TSAN to confirm
        for index, conn in enumerate(self.sockets):
            conn.recv(BUF_SIZE), OP_CODES["location_add"]

        # repeat for deletion
        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["location_del"], self.session_ids[index], 8, 0, "Pylandia", 0)
            conn.send(data)

        # nothing to really check here because you can't really know if it succeeded or not. must run with TSAN to confirm
        for index, conn in enumerate(self.sockets):
            conn.recv(BUF_SIZE), OP_CODES["location_del"]

    def test_4_location_list(self):
        """ List the same location many times at once"""

        location = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict.keys()))]

        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["list"], self.session_ids[index], len(location), 0, location, 0)
            conn.send(data)

        for index, conn in enumerate(self.sockets):
            packet = deserialize(conn.recv(BUF_SIZE), OP_CODES["list"])
            self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def test_5_location_list_diff(self):
        """ List DIFFERENT locations many times at once"""
        locations = list(uid_location_dict.keys())
        for index, conn in enumerate(self.sockets):
            data = serialize(OP_CODES["list"], self.session_ids[index], len(locations[index]), 0, locations[index], 0)
            conn.send(data)

        for index, conn in enumerate(self.sockets):
            packet = deserialize(conn.recv(BUF_SIZE), OP_CODES["list"])
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Didn't get list result for {locations[index]}")

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        for k,v in CREDENTIALS.items():
            send_recv(self.sockets[0], OP_CODES["user_del"], self.session_ids[0], len(k), 0,k,0)
        for i in self.sockets:
            i.close()

class RaceLinks(unittest.TestCase):
    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.sockets = []
        for _ in range(20):
            self.sockets.append(set_up())
    
    #TODO

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        for i in self.sockets:
            i.close()

class RaceRoute(unittest.TestCase):
    reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}

    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.sockets = []
        self.session_ids = []
        for _ in range(21):
            self.sockets.append(set_up())
    
        self.session_ids.append(admin_login(self.sockets[0]))
        count = 1
        # create every new user and then log them in on seperate file descriptors
        for k,v in CREDENTIALS.items():
            if count > 20:
                break
            send_recv(self.sockets[0], OP_CODES["register"], 0, len(k), len(v), k+v, 0)
            packet = send_recv(self.sockets[count], OP_CODES["login"], self.session_ids[0], len(k), len(v), k+v, 0)
            self.session_ids.append(packet.session_id)
            count += 1
        
    def test_1_route_all_at_once(self):
        """Test creates 20 connections and requests all the routes in batches of 20"""
        # (data, src, dst, dist, expected_path)
        contexts = []
        count = 0
        user_sessions = self.session_ids[1:]
        for src_loc, src_uid in uid_location_dict.items():
            if src_uid not in STARMAP.get_nodes():
                # edge case for nodes that don't exist in the map
                continue
            spf_map = dijkstra.DijkstraSPF(STARMAP, src_uid)
            for dst_uid in STARMAP.get_nodes():
                dst_loc = self.reverse_uid_dict[dst_uid]
                if src_loc == dst_loc:
                    # TODO: route to itself will be handled in another test?
                    continue
                expected_dist = spf_map.get_distance(dst_uid)
                data = serialize(OP_CODES["route"], user_sessions[count % len(user_sessions)], len(src_loc), len(dst_loc), src_loc+dst_loc, 0)
                count += 1
                contexts.append((data, src_loc, dst_loc, expected_dist))
        
        # send all the requests
        user_sockets = self.sockets[1:]
        for i, context in enumerate(contexts):
            user_sockets[i%len(user_sockets)].send(context[0])
            # check the returns for each batch to see if it gets the correct response
            if i == (len(user_sockets) - 1):
                for j, context_j in enumerate(contexts[i-(len(user_sockets)-1):i+1]):
                    response = deserialize(user_sockets[j%len(user_sockets)].recv(BUF_SIZE),OP_CODES["route"])
                    # context_j(data, src, dst, dist, expected_path)
                    if context_j[2] == inf:
                        self.assertEqual(response.ret_code, RETURN_CODES["unreachable"])    
                    else:
                        self.assertEqual(response.ret_code, RETURN_CODES["success"])
                        self.assertEqual(response.total_dist, context_j[3])
    
    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        for k,v in CREDENTIALS.items():
            send_recv(self.sockets[0], OP_CODES["user_del"], self.session_ids[0], len(k), 0,k,0)
        for i in self.sockets:
            i.close()

class RaceUsers(unittest.TestCase):
    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.sockets = []
        for _ in range(21):
            self.sockets.append(set_up())
    
        self.admin_session = admin_login(self.sockets[0])
        
    def test_1_many_register(self):
        """Test creates 20 connections and requests all the registers at once in batches of 20"""
        # create every new user and then log them in on seperate file descriptors
        data_list = []
        count = 0
        user_sockets = self.sockets[1:]
        for k,v in CREDENTIALS.items():
            count += 1
            data_list.append(serialize(OP_CODES["register"], 0, len(k), len(v), k+v, 0))
            if count % len(user_sockets) == 0:
                # send all data
                for i, data in enumerate(data_list):
                    user_sockets[i].send(data)
                # recv all data
                response = deserialize(user_sockets[i].recv(BUF_SIZE), OP_CODES["register"])
                self.assertEqual(response.ret_code, RETURN_CODES["success"])
                data_list = []
    
    def test_2_many_login(self):
        """Test creates 20 connections and requests to login at the same time on all of them"""
        self.test_1_many_register()
        data_list = []
        count = 0
        user_sockets = self.sockets[1:]
        for k,v in CREDENTIALS.items():
            count += 1
            data_list.append(serialize(OP_CODES["login"], 0, len(k), len(v), k+v, 0))
            if count % len(user_sockets) == 0:
                # send all data
                for i, data in enumerate(data_list):
                    user_sockets[i].send(data)
                # recv all data
                response = deserialize(user_sockets[i].recv(BUF_SIZE), OP_CODES["login"])
                self.assertEqual(response.ret_code, RETURN_CODES["success"])
                data_list = []

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        for k,v in CREDENTIALS.items():
            send_recv(self.sockets[0], OP_CODES["user_del"], self.admin_session, len(k), 0, k,0)
        for i in self.sockets:
            i.close()
