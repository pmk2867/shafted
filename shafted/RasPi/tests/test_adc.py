#!/usr/bin/python

# Python built-in modules.
import random
import unittest

# Custom-written modules.
import adc

class mock_device(object):
    """
    Simulate an ADC device to test with the ADC class.
    """
    def __init__(self, parameters):
        """
        Take in a tuple of parameters.

        :param parameters [tuple]: A tuple of any length.
        """
        self.parameters = list(parameters)

    def read(self):
        """
        Print an integer value.

        :return [int]: A random integer value.
        """
        return random.randint(-127, 127)

    def change_param(self, position, value):
        """
        Change a parameter of the device.

        :param position [int]: The index in the parameters list.
        :param value [int|float|str]: The new value to assign.
        """
        self.parameters[position] = value

class adcTest(unittest.TestCase):

    def setUp(self):
        """
        Setup the testcase by creating an ADC under test.
        """
        self.aut = adc.adc(mock_device((0, 1, 2, 3)))

    def test_read(self):
        """
        Make sure that the read operation correctly calls the child device's
        read() method.
        """
        self.assertIsInstance(self.aut.read(), int)

    def test_write(self):
        """
        Make sure that the change_param() method of the child device is properly
        called when the adc.write() method is called.
        """
        self.aut.write(0, 3)
        self.aut.write(1, 2)
        self.aut.write(2, 1)
        self.aut.write(3, 0)

        self.assertEqual(self.aut.device.parameters[0], 3)
        self.assertEqual(self.aut.device.parameters[1], 2)
        self.assertEqual(self.aut.device.parameters[2], 1)
        self.assertEqual(self.aut.device.parameters[3], 0)
