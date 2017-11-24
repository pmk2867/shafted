#!/usr/bin/python

# Python built-in modules
import math
import sys
import time
import unittest

# Project modules
import speed

class speedTest(unittest.TestCase):

    def setUp(self):
        """
        Set up the testcase by creating the unit under test, then set an
        acceptable tolerance, based on what operating system the unit test is
        running on.
        """
        self.dut = speed.speedCounter()

        self.isWindows = (sys.platform.startswith('win') or
                          sys.platform.startswith('cyg'))

        if self.isWindows:
            self.tolerance = 5 # In units of percent.
        else:
            self.tolerance = 1

    def do_delay(self, delay):
        """
        Execute a delay period. Different methods of executing the delay are
        used depending on the operating system the unit test is running on.

        :param delay [int|float] The duration in seconds to delay.
        """
        self.dut.trigger()

        timePrev = time.time()

        if self.isWindows:
            while(time.time() - timePrev < delay):
                continue
        else:
            time.sleep(delay)

        self.dut.trigger()

    def check_tolerance(self, value, expected):
        """
        Compare a certain value with the expected value and the given tolerance,
        defined in the testcase setup.

        :param value [int|float] The value to compare to the expected.
        :param expected [int|float] The expected value for this comparison.

        :return [bool] Whether or not the value is within tolerance of expected.
        """
        upper_bound = expected + (expected * (0.01 * self.tolerance))
        lower_bound = expected - (expected * (0.01 * self.tolerance))

        return value > lower_bound and value < upper_bound

    def test_trigger_10ms(self):
        """
        Test that the speedcounter can track a trigger with a 10ms period.
        """
        self.do_delay(0.010)

        self.assertTrue(self.check_tolerance(round(self.dut.get_hertz()), 100))
        self.assertTrue(self.check_tolerance(round(self.dut.get_per_minute()), 6000))
        self.assertTrue(self.check_tolerance(round(self.dut.get_per_hour()), 360000))

    def test_trigger_1s(self):
        """
        Test that the speedcounter can track a trigger with a longer period.
        """
        self.do_delay(1)

        self.assertTrue(self.check_tolerance(round(self.dut.get_hertz()), 1))
        self.assertTrue(self.check_tolerance(round(self.dut.get_per_minute()), 60))
        self.assertTrue(self.check_tolerance(round(self.dut.get_per_hour()), 3600))

    def tearDown(self):
        del self.dut
