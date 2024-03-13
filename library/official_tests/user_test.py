import unittest
from constants import *
from helpers import *


class LoginTests(unittest.TestCase):
    """
    Test cases for login functionality.
    """
    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.socket = set_up()
    
    def test_1_login_admin(self):
        """
        Test that an admin can log in with correct credentials.
        """
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 8, "adminpassword", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def test_2_login_dne(self):
        """
        Test login with various off-by-one credential errors and ensure they fail.
        """
        # check for some off-by-one comparisons of a good login
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 7,"adminpasswor", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="Bad logins should return 'credentials Errors'")
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 9,"adminpassword1", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="Bad logins should return 'credentials Errors'")
        packet = send_recv(self.socket, OP_CODES["login"], 0, 6, 8,"admin1password", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="Bad logins should return 'credentials Errors'")
        packet = send_recv(self.socket, OP_CODES["login"], 0, 4, 8,"admipassword", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="Bad logins should return 'credentials Errors'")
        packet = send_recv(self.socket, OP_CODES["login"], 0, 10, 11,"johnnytestpassword123", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="Bad logins should return 'credentials Errors'")

        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["credentials"])

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        self.socket.close()

class RegisterTests(unittest.TestCase):
    """
    Test cases for user registration functionality.
    """

    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.socket = set_up()

    def test_1_register_new(self):
        """
        Test registration of new users and ensure subsequent logins succeed.
        """
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["register"], 0, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to register user: {k} with pass: {v}")

        # Test their logins
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to login user: {k} with pass: {v}")
        
        # Ensure no off-by-x strncmp() is occuring
        for k,v in CREDENTIALS.items():
            # off-by-1 uname
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k)-1, len(v), k[:-1]+v, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

            # off-by-1 password
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k), len(v)-1, k+v[:-1], 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_2_register_duplicate(self):
        """
        Ensure duplicate usernames are rejected during registration.
        """
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["register"], 0, len(k), len(v), k+v, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_3_bad_register(self):
        """
        Test registration with invalid characters in username and password.
        """
        # test invalid characters
        # bad uname
        # non-ascii chars
        packet = send_recv(self.socket, OP_CODES["register"], 0, 5, 8, "us\x01\x021password", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Non-ascii username")
        # punctuation
        packet = send_recv(self.socket, OP_CODES["register"], 0, 5, 8, "u$er1password", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="punctuation username")
        # empty username
        packet = send_recv(self.socket, OP_CODES["register"], 0, 0, 8, "password", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="empty username")

        # bad pw
        packet = send_recv(self.socket, OP_CODES["register"], 0, 5, 8, "user2pa\x09\x0Aword", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Non-ascii password")
        # empty password
        packet = send_recv(self.socket, OP_CODES["register"], 0, 5, 0, "admin", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Empty Password")
    
    def test_4_cleanup(self):
        """
        Delete created users to clean up after tests.
        """
        # delete created users, note, i'll do an official test for this later
        # login as admin to get session id
        session_id = admin_login(self.socket)
        for k,_ in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k), 0, k, 0)
            self.assertIsNotNone(packet)


    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        self.socket.close()

class AdminAddTests(unittest.TestCase):
    """
    Test cases for adding admin users
    """

    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.socket = set_up()

    def test_1_add_normal(self):
        """
        Test adding new admin users and verify their permissions are correct.
        """
        # login as admin to get session id
        session_id = admin_login(self.socket)

        # Register new admins
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to add admin: {k} with pass: {v}")
        
        # check that login and perms are valid
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to login to admin: {k} with pass: {v}")
            new_session_id = packet.session_id
            
            # try an admin action
            packet = send_recv(self.socket, OP_CODES["admin_add"], new_session_id, 9, 8, "TestAdminpassword", 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"New admin: {k} unable to complete admin action: 'admin_add'")

            # delete the new user - will not work cuz all users logged in


            packet = send_recv(self.socket, OP_CODES["user_del"], new_session_id, 9, 0, "TestAdmin", 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Unable to delete the newly created admin: 'Test'")
        
    def test_2_add_duplicate(self):
        """
        Ensure that duplicate admin usernames are rejected.
        """
        # repeat admin name
        # login as admin to get session id
        session_id = admin_login(self.socket)

        # should be all duplicates now
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len(k), len(v), k+v, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
            # delete as we go
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k), 0, k, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Failed to delete user: {k}")

        # add all CREDENTIALS as user level
        RegisterTests.test_1_register_new(self)

        # re-login
        session_id = admin_login(self.socket)

        # Test if admins can be named the same as a taken username
        for k,v in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len(k), len(v), k+v, 0)
            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
            # delete as we go
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k),0, k, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Unable to delete newly created admin: {k}")
        
    def test_3_bad_perms(self):
        """
        Verify that users with insufficient permissions cannot add admins.
        """
        # register a normal user
        packet = send_recv(self.socket, OP_CODES["register"], 0, len("Alice"),
                           len(CREDENTIALS["Alice"]), f"Alice{CREDENTIALS['Alice']}", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg=f"Unable to register user: Alice")

        # login
        packet = send_recv(self.socket, OP_CODES["login"], 0, len("Alice"),
                           len(CREDENTIALS["Alice"]), f"Alice{CREDENTIALS['Alice']}", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        session_id = packet.session_id

        # Try and add an admin even with user privs
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len("Bob"),
                           len(CREDENTIALS["Bob"]), f"Bob{CREDENTIALS['Bob']}", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"])

        # login as admin to get session id
        session_id = admin_login(self.socket)

        # delete Alice
        packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len("Alice"),0, "Alice", 0)
        #self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def test_4_bad_chars_zero_name_len(self):
        """
        Test adding an admin with invalid characters in username and password.
        """
        session_id = admin_login(self.socket)

        # zero name len
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, 0,
                           len(CREDENTIALS["Bob"]), f"{CREDENTIALS['Bob']}", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_5_bad_chars_pw_len(self):
        """
        Test adding an admin with invalid characters in username and password.
        """
        session_id = admin_login(self.socket)
        # zero pw len
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len("Bob"), 0, f"Bob", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="Accepted zero password length")

        send_recv(self.socket, OP_CODES["user_del"], session_id, len("Bob"), 0, "Bob", 0)

    def test_6_bad_chars_punct(self):
        """
        Test adding an admin with invalid characters in username and password.
        """
        session_id = admin_login(self.socket)
        # punct in name
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len("B()b"), len(CREDENTIALS["Bob"]), f"B()b{CREDENTIALS['Bob']}", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
    
    def test_7_bad_chars_non_ascii_name(self):
        """
        Test adding an admin with invalid characters in username and password.
        """
        session_id = admin_login(self.socket)
        # non-ascii in name
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len("B\x09b"), len(CREDENTIALS["Bob"]), f"B\x09b{CREDENTIALS['Bob']}", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_8_bad_chars_non_ascii_pw(self):
        """
        Test adding an admin with invalid characters in username and password.
        """
        session_id = admin_login(self.socket)
        # non-ascii in pw
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, len("Bob1"), len(CREDENTIALS["Bob"])+1, f"Bob1{CREDENTIALS['Bob']}\x80", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
    
    def test_9_cleanup(self):
        """
        Clean up by deleting any users or admins created during testing.
        """
        RegisterTests.test_4_cleanup(self)

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        self.socket.close()

class UserDelTests(unittest.TestCase):
    """
    Test cases for deleting users.
    """

    def setUp(self) -> None:
        """
        Set up a socket connection for each test.
        """
        self.socket = set_up()

    def test_1_delete_normal(self):
        """
        Test normal deletion of users by an admin.
        *** look at test, make no sense
        """
        # register all the users
        RegisterTests.test_1_register_new(self)
        session_id = admin_login(self.socket)

        # admin tries to delete all the users
        for k, v in CREDENTIALS.items():
            # login as user
            packet = send_recv(self.socket, OP_CODES["login"], session_id, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"])
            session_id = packet.session_id # NOTE changed this line to give correct sessionid
            # admin deletes user
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k),0, k, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["success"])

            # ensure the deleted user can no longer complete actions
            for i, j in OP_CODES.items():
                # skip the admin only actions
                if i not in ADMIN_OPS:
                    # splitting this up by what takes 1 or 2 args because some students may choose
                    # to return invalid packet for data in "padding" fields or extra chars
                    # in the data fields even before checking session ID

                    # takes one argument
                    if i in SINGLE_ARG_OPS:
                        packet = send_recv(self.socket, j, user_session, len(k), 0, k, 0)
                        self.assertEqual(packet.ret_code, RETURN_CODES["session_id"])
                    # takes two argument
                    elif i in DUAL_ARG_OPS:
                        if i not in ["login", "register"]:
                            packet = send_recv(self.socket, j, user_session, len(k), len(v), k+v, 0)
                            self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])


            # login should now fail with invalid credentials returned
            packet = send_recv(self.socket, OP_CODES["login"], 0, len(k), len(v), k+v, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["credentials"])
    
    def test_2_delete_non_existent(self):
        """
        Ensure attempting to delete non-existent users fails appropriately.
        """
        session_id = admin_login(self.socket)

        # admin deletes users that don't exist should return "not_found"
        for k, _ in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k), 0, k, 0)
            self.assertEqual(packet.ret_code, RETURN_CODES["not_found"], msg=f"Didn't get 'not found' when deleting non-existent user: {k}")
    
    def test_3_delete_empty(self):
        """
        Test deletion behavior with empty usernames.
        """
        session_id = admin_login(self.socket)

        packet = send_recv(self.socket, OP_CODES["user_del"], session_id, 0, 0, "", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])
    
    def test_4_delete_bad_perms(self):
        """
        Ensure users cannot delete other users or admins without proper permissions.
        """
        packet = send_recv(self.socket, OP_CODES["register"], 0, 5, 19, "SpockLive long & prosper", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        packet = send_recv(self.socket, OP_CODES["register"], 0, 4, 12, "KirkEnterprise#1", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 19, "SpockLive long & prosper", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        new_session = packet.session_id

        # users shouldn't be able to delete other users
        packet = send_recv(self.socket, OP_CODES["user_del"], new_session, 4, 0, "Kirk", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"])

        # users shouldn't be able to delete admins
        packet = send_recv(self.socket, OP_CODES["user_del"], new_session, 5, 0, "admin", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"])

        # admin deletes them both now
        admin_session = admin_login(self.socket)

        # quick off-by-one check
        packet = send_recv(self.socket, OP_CODES["user_del"], admin_session, 4, 0, "Spoc", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["not_found"])
        packet = send_recv(self.socket, OP_CODES["user_del"], admin_session, 6, 0, "Spock2", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["not_found"])

        # now delete them
        packet = send_recv(self.socket, OP_CODES["user_del"], admin_session, 5, 0, "Spock", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        packet = send_recv(self.socket, OP_CODES["user_del"], admin_session, 4, 0, "Kirk", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # create another admin
        packet = send_recv(self.socket, OP_CODES["admin_add"], admin_session, 5, 8, "JamesTiberius", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        
        # ensure admin can be deleted by another admin
        packet = send_recv(self.socket, OP_CODES["user_del"], admin_session, 5, 0, "James", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        session_id = admin_login(self.socket)
        for k,_ in CREDENTIALS.items():
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, len(k), 0, k, 0)
            self.assertIsNotNone(packet)

        self.socket.close()

class UserPassTests(unittest.TestCase):
    """
    Test cases for changing user passwords.
    """

    def setUp(self):
        """
        Set up a socket connection for each test.
        """
        self.socket = set_up()

    def test_1_good_change_self(self):

        session_id = admin_login(self.socket)
        # admin can change self
        packet = send_recv(self.socket, OP_CODES["user_pass"], session_id, 5, 5, "adminadmin", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # confirm with login
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 5, "adminadmin", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        session_id = packet.session_id

        # change it back
        packet = send_recv(self.socket, OP_CODES["user_pass"], session_id, 5, 8, "adminpassword", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # confirm with login
        session_id = admin_login(self.socket)

    def test_2_good_change_other(self):
        """
        registers a new user, admin changes their password, confirm with logins, delete the new user
        """

        session_id = admin_login(self.socket)
        # create another user
        packet = send_recv(self.socket, OP_CODES["register"], 0, 7, 11, "SettledSwAmPl3t1cs", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # change others (as admin)
        packet = send_recv(self.socket, OP_CODES["user_pass"], session_id, 7, 6, "SettledMcTiLe", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="change others as admin")

        # original password should fail
        packet = send_recv(self.socket, OP_CODES["login"], 0, 7, 11, "SettledSwAmPl3t1cs", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="original password should fail")

        # new password should succeed
        packet = send_recv(self.socket, OP_CODES["login"], 0, 7, 6, "SettledMcTiLe", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="new password should succeed")

        session_id = admin_login(self.socket)
        
        # delete user
        packet = send_recv(self.socket, OP_CODES["user_del"], session_id, 7, 0, "Settled", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="Failed to cleanup and delete new user")

    def test_2_good_change_admin(self):
        session_id = admin_login(self.socket)

        # create admin
        packet = send_recv(self.socket, OP_CODES["admin_add"], session_id, 5, 10, "BoatyLowlife108", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # change new admin password as "admin"
        packet = send_recv(self.socket, OP_CODES["user_pass"], session_id, 5, 6, "BoatymaxUIM", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="change other admin as admin")

        # old password should fail
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 10, "BoatyLowlife108", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["credentials"], msg="old password should fail")

        # new password should succeed
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 6, "BoatymaxUIM", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="new password should succeed")

        session_id = admin_login(self.socket)
        # delete user
        packet = send_recv(self.socket, OP_CODES["user_del"], session_id, 5, 0, "Boaty", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="Failed to delete new test user")

    def test_2_permissions(self):
        """
        Verify that users can change their own password but not others' without permissions.
        """
        # create user
        packet = send_recv(self.socket, OP_CODES["register"], 0, 6, 7, "zezimaHunter2", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        packet = send_recv(self.socket, OP_CODES["login"], 0, 6, 7, "zezimaHunter2", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])
        user_session = packet.session_id

        # user can change self
        packet = send_recv(self.socket, OP_CODES["user_pass"], user_session, 6, 7, "zezimaHunter3", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # re-login
        packet = send_recv(self.socket, OP_CODES["login"], 0, 6, 7, "zezimaHunter3", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="Login with the newly changed password")
        user_session = packet.session_id

        # make another user
        packet = send_recv(self.socket, OP_CODES["register"], 0, 4, 7, "SoloMission", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["success"])

        # change others as user (should fail)
        packet = send_recv(self.socket, OP_CODES["user_pass"], user_session, 4, 8, "SoloChampion", 0)
        self.assertEqual(packet.ret_code, RETURN_CODES["permissions"])

    def test_3_user_dne(self):
        """
        Test changing passwords for non-existent users.
        """
        session_id = admin_login(self.socket)

        # non existent user
        packet = send_recv(self.socket, OP_CODES["user_pass"], session_id, 7, 4, "PatrickStar", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"])

    def test_4_bad_chars(self):
        """
        Test changing passwords with invalid characters.
        """
        # login as admin
        session_id = admin_login(self.socket)

        # invalid chars in password
        packet = send_recv(self.socket, OP_CODES["login"], session_id, 5, 8, "adminpas\x90word", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="FAILED: invalid chars in pw")

        packet = send_recv(self.socket, OP_CODES["login"], session_id, 5, 8, "adminpas\x09word", 0)
        self.assertNotEqual(packet.ret_code, RETURN_CODES["success"], msg="FAILED: invalid chars in pw")

    def tearDown(self):
        """
        Close the socket connection after each test.
        """
        # cleanup
        packet = send_recv(self.socket, OP_CODES["login"], 0, 5, 8, "adminpassword", 0)
        if packet:
            self.assertEqual(packet.ret_code, RETURN_CODES["success"], msg="Failed to login as admin")
            session_id = packet.session_id

            # delete users, dont check for success as they may or may not exist
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, 4, 0, "Solo", 0)
            packet = send_recv(self.socket, OP_CODES["user_del"], session_id, 6, 0, "zezima", 0)
        else:
            print("[!] Failed to cleanup during user_pass tests")
        self.socket.close()