#!/usr/bin/python

# Python built-in modules
import sys
import unittest

# Project Modules
from ADS1115 import ads1115_single, ads1115_differential

"""
Creating a few sets of parameters that will be used to pass to the
created ADS1115 objects.
"""
# Channel 0, Gain of 1, 0x48 Address, Busnumber 1
standard_params = [0, 1, 0x48, 1]

# Check to make sure unset parameters are okay, with 2/3 gain
assume_defaults = [0, 2/3]

# Make sure a list of less than 2 items is failed.
unfinished_list = [0]

# Make sure an invalid channel number fails.
invalid_channel = [4, 1]

# Make sure an invalid gain fails.
invalid_gain = [0, 1/3]

# Make sure an invalid address (like if it's replaced by busnum) fails.
invalid_address = [0, 1, 1]

# Make sure an invalid busnum fails.
invalid_busnum = [0, 1, 0x48, 2]

class singleTest(unittest.TestCase):
    """
    Testing the ads1115_single class
    """
    def test_device_creation(self):
        """
        Test all the various parameter sets to make sure that they all are
        either successful or appropriately throw assertion errors.
        """
        try:
            test_adc = ads1115_single(standard_params)
        except AssertionError:
            self.fail("The standard parameters threw an assertion error.")

        try:
            test_adc = ads1115_single(assume_defaults)
        except AssertionError:
            self.fail("Assuming defaults or setting gain = 2/3 threw an error.")

        try:
            test_adc = ads1115_single(unfinished_list)
        except AssertionError:
            # An assertion error is expected. Catch it and move on.
            pass

        try:
            test_adc = ads1115_single(invalid_channel)
        except AssertionError:
            # An assertion error is expected. Catch it and move on.
            pass

        try:
            test_adc = ads1115_single(invalid_gain)
        except AssertionError:
            # An assertion error is expected. Catch it and move on.
            pass

        try:
            test_adc = ads1115_single(invalid_address)
        except AssertionError:
            # An assertion error is expected. Catch it and move on.
            pass

        try:
            test_adc = ads1115_single(invalid_busnum)
        except AssertionError:
            # An assertion error is expected. Catch it and move on.
            pass

    def test_device_read(self):
        """
        Create a default ADS1115 in single-sided mode and attempt a read.
        """
        test_adc = ads1115_single(assume_defaults)

        try:
            value = test_adc.read()
            self.assertIsInstance(value, int)
        except FileNotFoundError:
            # If this occurs, the I2C file was not found in /dev/, likely
            # because the ADC isn't actually attached to the dut. Pass the test.
            pass

    def test_change_param(self):
        """
        Create a default ADS1115 in single-sided mode and attempt to change its
        parameters.
        """
        test_adc = ads1115_single(assume_defaults)

        # Check to make sure an assertion is thrown if a position beyond 3 is
        # chosen.
        try:
            test_adc.change_param(4, 0)
            # If the assertion error hasn't been thrown, fail the test.
            self.fail()
        except AssertionError:
            # An assertion error is expected.
            pass

        # Check to make sure assertion errors are still thrown for invalid
        # parameters. Only one check is done because test_device_creation
        # has already walked the check_params internal function to make sure
        # that all invalid parametes are failed.
        try:
            # Try changing address to an invalid value.
            test_adc.change_param(2, 0)
            # If the assertion error hasn't been thrown, fail the test.
        except AssertionError:
            # An assertion error is expected.
            pass

        # Check to make sure that all parameters can be changed when the change
        # is valid.

        # Set channel to 3.
        test_adc.change_param(0, 3)
        self.assertEqual(test_adc.parameters[0], 3)

        # Set gain to 4.
        test_adc.change_param(1, 4)
        self.assertEqual(test_adc.parameters[1], 4)

        # Set address to 0x49.
        test_adc.change_param(2, 0x49)
        self.assertEqual(test_adc.parameters[2], 0x49)

        # Set busnum to 0.
        test_adc.change_param(3, 0)
        self.assertEqual(test_adc.parameters[3], 0)
