import unittest
import dijkstra
import random
from constants import *
from helpers import *
from math import inf


STARMAP = build_graph(MAP_PATH)

class ListTests(unittest.TestCase):
    def setUp(self):
        self.socket = set_up()
    
    def check_list(self, session_id):
        # without param
        packet = send_recv(self.socket, OP_CODES["list"], session_id, 0, 0, "", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="List without params should return success and all the nodes")
        locations = [part.decode('utf-8') for part in packet.data.split(b'\0')]
        for k in uid_location_dict.keys():
            self.assertIn(k, locations, msg=f"{k} not found in `list` command")

        # with param
        for location_name, location_uid in uid_location_dict.items():
            packet = send_recv(self.socket, OP_CODES["list"], session_id, len(location_name), 0, location_name, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to list adjacencies for: {location_name}")
            expected = []
            reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}
            for uid in STARMAP.get_adjacent_nodes(location_uid):
                # make a list of the expected names
                expected.append(reverse_uid_dict[uid])
            received = []
            if len(packet.data):
                received = [part.decode('utf-8') for part in packet.data.split(b'\0')]
            try:
                expected.remove(location_name)
            except ValueError:
                pass
            try:
                received.remove(location_name)
            except ValueError:
                pass
            self.assertEqual(len(received), len(expected), msg=f"List fail, planet: {location_name}. Expected: {expected}, Received: {received}")
            for i in expected:
                self.assertIn(i, received, msg=f"{i} missing from data: {received}")

    def test_1_good(self):
        """
        Test normal list behavior
        """
        admin_session = admin_login(self.socket)

        # as admin w and w/o params
        self.check_list(admin_session)

        # create new user and login
        send_recv(self.socket, OP_CODES["register"], 0, 4, 4, "SoupOSRS", 0)
        packet = send_recv(self.socket, OP_CODES["login"], 0, 4, 4, "SoupOSRS", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        user_session = packet.session_id

        # as user w and w/o param
        self.check_list(user_session)

        # remove the user
        send_recv(self.socket, OP_CODES["user_del"], admin_session, 4, 0, "Soup", 0)

    def test_2_bad_session(self):
        admin_login(self.socket)
        packet = send_recv(self.socket, OP_CODES["list"], 0, 0, 0, "", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["session_id"])
    
    def test_3_location_dne(self):
        admin_session = admin_login(self.socket)
        bad_location = "Arrakis"

        packet = send_recv(self.socket, OP_CODES["list"], admin_session, len(bad_location), 0, bad_location, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Able to list with a location that doesn't exist")

    def tearDown(self):
        self.socket.close()

class SearchTests(unittest.TestCase):
    # [id] = name
    reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}

    def setUp(self):
        self.socket = set_up()

    def test_1_good_search(self):
        """
        Test for known good search with a full match
        """
        # as admin
        admin_session = admin_login(self.socket)

        # TODO: possible bug if a location is a subset of multiple locations
        # example: Earth and Earth-Prime or something
        name = self.reverse_uid_dict[2]
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        data = [part.decode('utf-8') for part in packet.data.split(b'\0')]
        for i in data:
            self.assertIn(name, i, msg=f"Searching for {name} returned {i} in search results")

    def test_2_multiple_matches(self):
        """
        Test for multiple match search strings
        """
        # as admin
        admin_session = admin_login(self.socket)

        # just the first two letters of a random name in the dict
        name = list(uid_location_dict.keys())[random.randrange(0,len(uid_location_dict))][:2]
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        data = [part.decode('utf-8') for part in packet.data.split(b'\0')]
        for i in data:
            self.assertIn(name, i, msg=f"Searching for {name} returned {i} in search results")

        # try for middle two letters
        # TODO: bug if length is 2 or less already

        name = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))][2:-2]
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        data = [part.decode('utf-8') for part in packet.data.split(b'\0')]
        expected =[]
        for i in list(uid_location_dict.keys()):
            if name in i:
                expected.append(i)

        self.assertEqual(len(expected), len(data), msg=f"Received {len(data)} matches, but expected: {len(expected)}.\n Expected:{expected}\n\nRecieved: {data}")
        for i in data:
            self.assertIn(name, i)

        name = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))][1:-2]
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        data = [part.decode('utf-8') for part in packet.data.split(b'\0')]
        expected =[]
        for i in uid_location_dict.keys():
            if name in i:
                expected.append(i)
        
        self.assertEqual(len(expected), len(data))
        for i in data:
            self.assertIn(name, i)

    def test_3_no_match(self):
        # as admin
        admin_session = admin_login(self.socket)

        name = "abcdefghijklmnopqrstuvwxyz12345"
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        if packet.ret_code == RETURN_CODES["success"]:
            self.assertEqual(packet.data_len, 0)
            self.assertEqual(len(packet.data), 0)

    def test_4_empty(self):
        admin_session = admin_login(self.socket)
        # TODO: Should this return everything? or nothing? for now, don't check
        # the result and just see if it crashes stuff
        name = ""
        send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        

    def test_5_non_ascii(self):
        admin_session = admin_login(self.socket)

        name = "\xA0\xB0\x90"
        packet = send_recv(self.socket, OP_CODES["search"], admin_session, len(name), 0, name, 0)
        if packet.ret_code == RETURN_CODES["success"]:
            self.assertEqual(packet.data_len, 0)
        else:
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        self.socket.close()

class RouteTests(unittest.TestCase):
    reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}

    def setUp(self):
        self.socket = set_up()
        self.admin_session = admin_login(self.socket)

    def test_1_direct_route(self):
        """
        Test just the direct paths
        """
        # Check all edges for all nodes for all direct paths
        count = 0
        node_count = len(uid_location_dict)
        for src_loc, uid in uid_location_dict.items():
            for adj_node in STARMAP.get_adjacent_nodes(uid):
                count +=1
                if node_count // 10 == count:
                    print(".",end="")
                dst_loc = self.reverse_uid_dict[adj_node]
                if src_loc == dst_loc:
                    continue
                spf_node = dijkstra.DijkstraSPF(STARMAP, uid_location_dict[src_loc])
                expected = spf_node.get_distance(uid_location_dict[dst_loc])
                packet = send_recv(self.socket, OP_CODES["route"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 0)
                self.assertEqual(packet.ret_code, RETURN_CODES["success"])
                self.assertEqual(packet.total_dist, expected, msg=f"Distance from {src_loc} to {dst_loc} expected: {expected}, got: {packet.total_dist}")

    def test_2_routes(self):
        """
        Test up to 100 nodes and their routes
        """
        nodes = STARMAP.get_nodes()
        # Convert uid_location_dict.items() to a list and sample up to 100 items.
        # If there are fewer than 100 items, it samples all of them.
        sampled_items = random.sample(list(uid_location_dict.items()), min(len(uid_location_dict), 100))

        count = 0
        node_count = len(sampled_items)
        for src_loc, src_uid in sampled_items:
            if src_uid not in nodes:
                # edge case for nodes that don't exist in the map
                continue
            spf_map = dijkstra.DijkstraSPF(STARMAP, src_uid)
            count +=1
            if node_count // 10 == count:
                print(".",end="",flush=True)
            for dst_uid in nodes:
                dst_loc = self.reverse_uid_dict[dst_uid]
                if src_loc == dst_loc:
                    # TODO: route to itself will be handled in another test?
                    continue
                expected_dist = spf_map.get_distance(dst_uid)
                packet = send_recv(self.socket, OP_CODES["route"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 0)
                if expected_dist == inf:
                    self.assertEqual(packet.ret_code, RETURN_CODES["unreachable"])
                else:
                    uid_path = spf_map.get_path(dst_uid)
                    expected_path = []
                    for uid in uid_path:
                        expected_path.append(self.reverse_uid_dict[uid])
                    self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to resolve route from {src_loc} to {dst_loc}")
                    received_path = [part.decode('utf-8') for part in packet.data.split(b'\0')]
                    self.assertEqual(packet.total_dist, expected_dist, msg=f"Bad distance calculation, got: {packet.total_dist}, expected: {expected_dist}")
                    # handles edge case where two routes are equal len but different routes
                    dist = 0
                    for i,v in enumerate(received_path):
                        try:
                            dist += STARMAP.get_edge_weight(uid_location_dict[v], uid_location_dict[received_path[i+1]])
                        except IndexError:
                            pass
                    self.assertEqual(dist, expected_dist, msg=f"route dist of {received_path} doesn't match real cost of {dist}")

    def test_3_locations_dne(self):
        """
        Test locations that don't exist
        """
        admin_session = admin_login(self.socket)
        bad_location = "Arrakis"
        start_location = list(uid_location_dict.keys())[0]

        packet = send_recv(self.socket, OP_CODES["route"], admin_session, len(start_location), len(bad_location), start_location+bad_location, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Routing to a location that doesn't exist returned success")

    def test_4_no_route(self):
        """
        Test impossible route, but locations do exist
        """
        admin_session = admin_login(self.socket)

        # add a planet but don't add any links
        bad_location = "Arrakis"
        packet = send_recv(self.socket, OP_CODES["location_add"], admin_session, len(bad_location), 0, bad_location, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        start_location = list(uid_location_dict.keys())[0]
        packet = send_recv(self.socket, OP_CODES["route"], admin_session, len(start_location), len(bad_location), start_location+bad_location, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["unreachable"])

        # remove it
        packet = send_recv(self.socket, OP_CODES["location_del"], admin_session, len(bad_location), 0, bad_location, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        self.socket.close()

class LinkTests(unittest.TestCase):
    reverse_uid_dict = {v: k for k, v in uid_location_dict.items()}

    def setUp(self):
        self.socket = set_up()
        self.admin_session = admin_login(self.socket)
    
    def add_del_random_link(self):
        src_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        dst_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        old_distance = 0
        # get a random source and destination location and ensure there is some
        # non-zero distance between them
        while old_distance == 0:
            dst_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
            if dst_loc != src_loc:
                spf_map = dijkstra.DijkstraSPF(STARMAP, uid_location_dict[src_loc])
                old_distance = spf_map.get_distance(uid_location_dict[dst_loc])

        # create a direct link of weight old_distance - 1
        # this is guaranteed to now be the shortest path
        if old_distance == inf:
            new_distance = 42
        else:
            new_distance = old_distance - 1
        
        packet = send_recv(self.socket, OP_CODES["link_add"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, new_distance)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to add link: {src_loc} -> {dst_loc} with weight={new_distance}")

        # check if it can now route there
        packet = send_recv(self.socket, OP_CODES["route"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        self.assertEqual(packet.total_dist, new_distance)
        # direct link has zero stops, just start and end, so two entries
        self.assertEqual(len(packet.data.split(b"\0")), 2)

        # remove the new link
        packet = send_recv(self.socket, OP_CODES["link_del"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, new_distance)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # route again to see if you get the old distance
        packet = send_recv(self.socket, OP_CODES["route"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 0)
        
        if old_distance == inf:
            self.assertEqual(packet.ret_code, RETURN_CODES["unreachable"])
        else:
            self.assertEqual(packet.ret_code, RETURN_CODES["success"])
            self.assertEqual(packet.total_dist, old_distance)

    def test_1_add_del_link_good(self):
        """Add 50 short links between random nodes, route to them, and then delete them"""
        for _ in range(50):
            self.add_del_random_link()

    def test_3_add_bad_link(self):
        """Add a link with a src or dst that doesn't exist
        """
        # link to self?

        # Attempt link to bad location
        src_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        dst_loc = "Gielinor"
        packet = send_recv(self.socket, OP_CODES["link_add"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 1)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Allows links to bad locations")

        # Attempt link from bad start
        packet = send_recv(self.socket, OP_CODES["link_add"], self.admin_session, len(dst_loc), len(src_loc), dst_loc+src_loc, 1)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Allows links from bad locations")

    def test_4_del_bad_link(self):
        """ Attempt to delete a link that doesn't exist"""
        # Get a random node's adjacents
        src_uid = list(uid_location_dict.values())[random.randrange(0, len(uid_location_dict))]
        adjacents = STARMAP.get_adjacent_nodes(src_uid)
        
        # find a uid NOT in it's adjacents
        missing_uid = 0
        for i in uid_location_dict.values():
            if i not in adjacents:
                missing_uid = i
                break
        
        dst_loc = self.reverse_uid_dict[missing_uid]
        src_loc = self.reverse_uid_dict[src_uid]

        # attempt to delete a link that doesn't exist
        packet = send_recv(self.socket, OP_CODES["link_del"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 1)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="deleted a link that doesn't exist")

        # attempt to delete a link that does exist, but is bad weight
        spf_map = dijkstra.DijkstraSPF(STARMAP, src_uid)
        for i in adjacents:
            dst_loc = self.reverse_uid_dict[i]
            # hopefully 65534 is never an actually valid weight
            packet = send_recv(self.socket, OP_CODES["link_del"], self.admin_session, len(src_loc), len(dst_loc), src_loc+dst_loc, 65534)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="deleted a link that doesn't exist (bad weight specified)")

    def test_5_perms(self):
        """ Try to add/del a link as a non-admin"""

        packet = send_recv(self.socket, OP_CODES["register"], 0, len("Johnny"), len("Test"), "JohnnyTest", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        packet = send_recv(self.socket, OP_CODES["login"], 0, len("Johnny"), len("Test"), "JohnnyTest", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        user_session = packet.session_id

        src_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        dst_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        old_distance = 0
        # get a random source and destination location and ensure there is some
        # non-zero distance between them
        while old_distance == 0:
            dst_loc = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
            if dst_loc != src_loc:
                spf_map = dijkstra.DijkstraSPF(STARMAP, uid_location_dict[src_loc])
                old_distance = spf_map.get_distance(uid_location_dict[dst_loc])
        new_distance = old_distance - 1
        if old_distance == inf:
            new_distance = 100
        # create a direct link of weight old_distance - 1
        # this is guaranteed to now be the shortest path
        packet = send_recv(self.socket, OP_CODES["link_add"], user_session, len(src_loc), len(dst_loc), src_loc+dst_loc, new_distance)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"], msg="User was allowed to complete admin action (link_add)")

        # remove the new link
        packet = send_recv(self.socket, OP_CODES["link_del"], user_session, len(src_loc), len(dst_loc), src_loc+dst_loc, new_distance)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"], msg="User was allowed to complete admin action (link_del)")

        self.admin_session = admin_login(self.socket)
        # delete the new user
        packet = send_recv(self.socket, OP_CODES["user_del"], self.admin_session, len("Johnny"), 0, "Johnny", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="failed to cleanup and delete user")

    def tearDown(self):
        self.socket.close()

class LocationTests(unittest.TestCase):
    def setUp(self):
        self.socket = set_up()
        self.admin_session = admin_login(self.socket)

    def test_1_add_del_good(self):
        """Create 50 random location names some with alpha, alphanum and alphanum+punct add them, then delete them"""
        list_size = 50
        planet_list = []
        # Find some locations that don't exist and add them to our list(s)
        for i in range(list_size):
            size = random.randrange(1,33)
            if i < 10:
                planet_list.append(generate_unique(generate_alpha, list(uid_location_dict.keys()) + planet_list, size))
            elif i < 20:
                planet_list.append(generate_unique(generate_alphanumeric, list(uid_location_dict.keys()) + planet_list, size))
            else:
                planet_list.append(generate_unique(generate_alphanumeric_punctuation, list(uid_location_dict.keys()) + planet_list, size))
        
        for planet in planet_list:
            # add all the planets
            packet = send_recv(self.socket, OP_CODES["location_add"], self.admin_session, len(planet), 0, planet, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to add planet: {planet}")
        
        for planet in planet_list:
            # delete all the planets
            packet = send_recv(self.socket, OP_CODES["location_del"], self.admin_session, len(planet), 0, planet, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to del planet: {planet}")

    def test_3_add_bad(self):
        """ Add a location that already exists"""
        # add a location that already exists
        random_planet = list(uid_location_dict.keys())[random.randrange(0, len(uid_location_dict))]
        packet = send_recv(self.socket, OP_CODES["location_add"], self.admin_session, len(random_planet), 0, random_planet, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to add a planet that exists already: {random_planet}")

    def test_3_add_bad_chars(self):
        """add a location that has illegal chars"""
        bad_name = "Bizaro \x03World"
        packet = send_recv(self.socket, OP_CODES["location_add"], self.admin_session, len(bad_name), 0, bad_name, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to add a planet with illegal chars in name")

    def test_4_add_bad_long(self):
        """add a location that has too large a name"""
        large_name = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        packet = send_recv(self.socket, OP_CODES["location_add"], self.admin_session, len(large_name), 0, large_name, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to add a planet with name too large: {len(large_name)} characters")

    def test_3_add_bad_empty(self):
        """add a location with empty name"""
        empty_name = ""
        packet = send_recv(self.socket, OP_CODES["location_add"], self.admin_session, len(empty_name), 0, empty_name, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to add a planet with name_len of 0")

    def test_4_del_bad(self):
        """Delete a location that doesn't exist"""
        random_string = generate_unique(generate_alphanumeric_punctuation, uid_location_dict.keys())

        packet = send_recv(self.socket, OP_CODES["location_del"], self.admin_session, len(random_string), 0, random_string, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to del a planet that doesn't exist")

    def test_4_del_bad_large(self):
        """Name too large (limit is 32)"""
        large_name = generate_alpha(33)
        packet = send_recv(self.socket, OP_CODES["location_del"], self.admin_session, len(large_name), 0, large_name, 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Able to del a planet with name too large")

    def test_5_perms(self):
        """register a user and then attempt to add/del locations as a non-admin"""
        packet = send_recv(self.socket, OP_CODES["register"], 0, len("Virtuoso"), len("Tanzoo"), "VirtuosoTanzoo", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        packet = send_recv(self.socket, OP_CODES["login"], 0, len("Virtuoso"), len("Tanzoo"), "VirtuosoTanzoo", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        user_session = packet.session_id

        # Make 30 random locations of varying complexity
        location_list = []
        for i in range(30):
            if i < 10:
                identifier = generate_unique(generate_alpha, list(uid_location_dict.keys()) + location_list)
            elif i < 20:
                identifier = generate_unique(generate_alphanumeric, list(uid_location_dict.keys()) + location_list)
            else:
                identifier = generate_unique(generate_alphanumeric_punctuation, list(uid_location_dict.keys()) + location_list)
            location_list.append(identifier)

        # user tries the admin action of adding one 
        for planet in location_list:
            packet = send_recv(self.socket, OP_CODES["location_add"], user_session, len(planet), 0, planet, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Non-Admin was allowed to add location: {planet}")

        # user tries to del known locations
        for location in uid_location_dict.keys():
            packet = send_recv(self.socket, OP_CODES["location_del"], user_session, len(location), 0, location, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["permissions"], msg=f"Non-Admin was allowed to del location: {location}")


        self.admin_session = admin_login(self.socket)
        # delete the new user
        packet = send_recv(self.socket, OP_CODES["user_del"], self.admin_session, len("Virtuoso"), 0, "Virtuoso", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        self.socket.close()
