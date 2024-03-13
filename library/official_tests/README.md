# Test Suite

## Install Requirements

`pip install -r requirements.txt`

## Run

1. from `starbase-1337` root
2. `python3 official_tests/main.py`

## usage

`usage: main.py [-h] [-i IP] [-p PORT] [-6] [-s] [-t TIMEOUT] [-m MAP]`

## user_test

1. **LoginTests**
   - `test_1_login_admin`: Test that an admin can log in with correct credentials.
   - `test_2_login_dne`: Test login with various off-by-one credential errors and ensure they fail.

2. **RegisterTests**
   - `test_1_register_new`: Test registration of new users and ensure subsequent logins succeed.
   - `test_2_register_duplicate`: Ensure duplicate usernames are rejected during registration.
   - `test_3_bad_register`: Test registration with invalid characters in username and password.
   - `test_4_cleanup`: Delete created users to clean up after tests.

3. **AdminAddTests**
   - `test_1_add_normal`: Test adding new admin users and verify their permissions are correct.
   - `test_2_add_duplicate`: Ensure that duplicate admin usernames are rejected.
   - `test_3_bad_perms`: Verify that users with insufficient permissions cannot add admins.
   - `test_4_bad_chars`: Test adding an admin with invalid characters in username and password.
   - `test_5_cleanup`: Clean up by deleting any users or admins created during testing.

4. **UserDelTests**
   - `test_1_delete_normal`: Test normal deletion of users by an admin.
   - `test_2_delete_non_existent`: Ensure attempting to delete non-existent users fails appropriately.
   - `test_3_delete_empty`: Test deletion behavior with empty usernames.
   - `test_4_delete_bad_perms`: Ensure users cannot delete other users or admins without proper permissions.

5. **UserPassTests**
   - `test_1_good_change_self`: Test that users can change their own passwords successfully.
   - `test_2_good_change_other`: Test that admins can change other users' passwords successfully.
   - `test_2_good_change_admin`: Test changing passwords for admin users.
   - `test_2_permissions`: Verify that users can change their own password but not others' without permissions.
   - `test_3_user_dne`: Test changing passwords for non-existent users.
   - `test_4_bad_chars`: Test changing passwords with invalid characters.

## star_test

1. **ListTests**
   - `test_1_good`: Tests the normal behavior of the list function, including admin and user sessions, with and without parameters.
   - `test_2_bad_session`: Tests the list function with an invalid session ID.
   - `test_3_location_dne`: Tests the list function with a location that does not exist.

2. **SearchTests**
   - `test_1_good_search`: Tests the search function with a full match known to be good.
   - `test_2_multiple_matches`: Tests the search function for handling multiple matches and various match patterns.
   - `test_3_no_match`: Tests the search function with a query expected to match no locations.
   - `test_4_empty`: Tests the search function with an empty query.
   - `test_5_non_ascii`: Tests the search function with non-ASCII characters.

3. **RouteTests**
   - `test_1_direct_route`: Tests direct paths between locations.
   - `test_2_routes`: Tests routes between up to 100 nodes, verifying distances and path correctness.
   - `test_3_locations_dne`: Tests routing to a location that doesn't exist.
   - `test_4_no_route`: Tests routing between locations where no route is possible.

4. **LinkTests**
   - `test_1_add_del_link_good`: Adds and then deletes 50 short links between random nodes and checks routing.
   - `test_3_add_bad_link`: Tries to add links with a source or destination that doesn't exist.
   - `test_4_del_bad_link`: Attempts to delete a non-existent link or one with incorrect parameters.
   - `test_5_perms`: Verifies that non-admin users cannot add or delete links.

5. **LocationTests**
   - `test_1_add_del_good`: Tests adding and deleting 50 random location names with various character sets.
   - `test_3_add_bad`: Attempts to add a location that already exists.
   - `test_3_add_bad_chars`: Attempts to add a location with illegal characters.
   - `test_4_add_bad_long`: Attempts to add a location with a name longer than the allowed limit.
   - `test_3_add_bad_empty`: Attempts to add a location with an empty name.
   - `test_4_del_bad`: Attempts to delete a non-existent location.
   - `test_4_del_bad_large`: Attempts to delete a location with a name longer than the allowed limit.
   - `test_5_perms`: Verifies that non-admin users cannot add or delete locations.

## edge_test

1. **BadPacketTests**
   - `test_1_data1_short`: Tests with data length one character short and zero length with data, expecting invalid return codes.
   - `test_2_data2_short`: Similar to `test_1_data1_short`, but for the second data argument, expecting invalid return codes.
   - `test_3_data1_long`: Tests with data1 length longer than actual, expecting invalid return codes.
   - `test_4_data2_long`: Tests with data2 length significantly longer than actual, expecting invalid return codes.
   - `test_5_both_long`: Tests with both data1 and data2 lengths significantly longer than actual, expecting invalid return codes.
   - `test_6_both_short`: Tests with both data1 and data2 lengths shorter than actual, expecting invalid return codes.
   - `test_7_bad_read`: Tests with extremely large length values, expecting invalid return codes.
   - `test_8_overflow`: Tests with data lengths that would cause an overflow, expecting invalid return codes.

2. **PartialSendRecvTests**
   - `test_1_partial_send`: Tests the server's handling of partial sends, ensuring that partial messages are correctly reassembled and processed.

3. **SuddenDisconnect**
   - `test_1_disconnect`: Tests the server's handling of a sudden disconnect after data has been sent.

## thread_test


1. **RaceLocations**
   - `test_1_location_add_del_good`: Tests concurrent addition and deletion of different locations by 20 connections.
   - `test_2_location_add_del_bad`: Tests concurrent addition and deletion of the same location by 20 connections, mainly for thread safety analysis (TSAN) verification.
   - `test_4_location_list`: Lists the same location multiple times concurrently from 20 connections.
   - `test_5_location_list_diff`: Lists different locations concurrently from 20 connections, verifying success for each.

2. **RaceLinks**
   - Currently, no tests are implemented in the `RaceLinks` class. It is set up for future expansion.

3. **RaceRoute**
   - `test_1_route_all_at_once`: Creates 20 connections and requests all routes in batches of 20, checking for correct responses including unreachable and success cases.

4. **RaceUsers**
   - `test_1_many_register`: Concurrently registers multiple users across 20 connections, verifying successful registrations.
   - `test_2_many_login`: After registrations, concurrently logs in the same users across 20 connections, verifying successful logins.
