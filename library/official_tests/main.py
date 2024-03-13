#!/usr/bin/env python3

import unittest
import argparse
from socket import AF_INET, AF_INET6
import constants


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run tests with customizable settings.")
    parser.add_argument('-i', '--ip', type=str, default=constants.IP_ADDRESS, help='IP address (default: 127.0.0.1)')
    parser.add_argument('-p', '--port', type=int, default=constants.PORT, help='Port number (default: 8000)')
    parser.add_argument('-6', '--ipv6', action='store_true', help='IP domain becomes IPv6')
    parser.add_argument('-s','--encrypted', action='store_true', help='Use encryption (default: False)')
    parser.add_argument('-t','--timeout', type=int, default=constants.TIMEOUT, help='Timeout in seconds')
    parser.add_argument('-m','--map', type=str, default=constants.MAP_PATH, help='path to mapfile')
    args = parser.parse_args()

    # Update constants
    constants.IP_ADDRESS = args.ip
    constants.PORT = args.port
    if args.ipv6:
        constants.IP_DOMAIN = AF_INET6
    constants.ENCRYPTED = args.encrypted
    constants.TIMEOUT = args.timeout
    constants.MAP_PATH = args.map

    loader = unittest.TestLoader()
    # Adding them manually to enforce an order to run the tests. It doesn't make 
    # sense to break their clients with the volume/edge tests before confirming
    # basic functionality
    test_suites = ["user_test.py", "stars_test.py", "edge_test.py", "volume_test.py", "thread_test.py"]
    test_suites = ["volume_test.py"]
    combined_suite = unittest.TestSuite()
    for suite in test_suites:
        temp = loader.discover(start_dir="official_tests", pattern=suite)
        combined_suite.addTests(temp)

    runner = unittest.TextTestRunner(verbosity=3)
    result = runner.run(combined_suite)

    passed = result.testsRun - (len(result.errors) + len(result.failures))
    summary = f"""
Summary:
Ran: {result.testsRun} tests
Errors: {len(result.errors)}
Failures: {len(result.failures)}
Passed: {passed}
"""
    print(summary)

