#!/usr/bin/python

import time

class adc(object):
    """
    Implements an object that can take in an analog voltage and convert to
    a digital representation of that voltage. The object should have only one
    channel.
    """

    def __init__(self, device):
        """
        Initializes an ADC with the type of device to use. The device should be
        some type of device that has a driver written for it, with abstract
        read() and change_param(position, value) methods.

        :param device: An object that can be written to and read from.
        :param parameters [list]: A list of parameters that should be written to
                                  the instantiated device.
        """
        self.device = device

    def read(self):
        """
        Reads the ADC.

        :return value [int]: Signed integer representation of the ADC value.
        """
        return self.device.read()

    def write(self, position, value):
        """
        Change one of the parameters set when the ADC was instantiated by
        defining the position in the original list occupied by the value, then
        providing the new value.

        :param position [int]: The position in the original list that held the
                               value wanting to be changed, indexed from 0.
        :param value [int|float|str]: The new value to change to.
        """
        self.device.change_param(position, value)
