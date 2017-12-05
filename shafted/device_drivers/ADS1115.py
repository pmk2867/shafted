#!/usr/bin/python

import Adafruit_ADS1x15

def check_params(parameters):
    """
    Check that the parameters passed to an ADS1x15 object contains the correct
    parameters.

    :param parameters [list]: Should contain all parameters in this order:
                               (channel, gain, address, busnum)
                               Where the address and busnum are optional.
    """
    # Require that at least two parameters (a channel and gain) are passed.
    assert len(parameters) >= 2

    # Make sure that a non-existent channel hasn't been selected.
    assert parameters[0] <= 3

    # Make sure that an unacceptable gain hasn't been selected.
    assert parameters[1] in [2/3, 1, 2, 4, 8, 16]

    try:
        # If 3rd parameter is passed, make sure that it's a valid address.
        assert parameters[2] in [0x48, 0x49, 0x4A, 0x4B]
    except IndexError:
        pass

    try:
        # If 4th parameter is passed, make sure that it's a valid busnum
        assert parameters[3] in [0, 1]
    except IndexError:
        pass

class ads1115_single(object):
    """
    Implements a single-ended channel of the ADS1115 chip, measured against the
    the chip's ground.
    """

    def __init__(self, parameters):
        """
        Initialize the single-ended channel of the ADS1115 chip.

        :param parameters [tuple]: Tuple of parameters (channel,
                                                        gain,
                                                        address,
                                                        busnum)
        """
        self.parameters = list(parameters)
        check_params(self.parameters)

        try:
            self.parameters[2]
        except IndexError:
            self.parameters.append(0x48)

        try:
            self.parameters[3]
        except IndexError:
            self.parameters.append(1)

        self.adc = Adafruit_ADS1x15.ADS1115(address=self.parameters[2],
                                            busnum=self.parameters[3])

    def read(self):
        """
        Read a value back from the ADC in the prior-configured channel with the
        prior-configured gain.

        :return [int]: ADC read value
        """
        return self.adc.read_adc(self.parameters[0], gain=self.parameters[1])

    def change_param(self, position, value):
        """
        Change a parameter in the list of parameters and update the list.

        :param position [int]: Some value [0, 3] to indicate position in list.
        :param value [int|float]: The new value to place in that position.
        """
        assert position in range(4)

        self.parameters[position] = value

        check_params(self.parameters)

        if position >= 2:
            # Rebuild the adc device only if the address or busnum have changed.
            self.adc = Adafruit_ADS1x15.ADS1115(address=self.parameters[2],
                                                busnum=self.parameters[3])


class ads1115_differential(object):
    """
    Implements a single-ended channel of the ADS1115 chip, measured against the
    the chip's ground.
    """

    def __init__(self, parameters):
        """
        Initialize the differential channel of the ADS1115 chip.

        On this configuration, the channel defines which two inputs on the ADC
        are being compared.

        Channel 0 - A0 Minus A1
        Channel 1 - A0 Minus A3
        Channel 2 - A1 Minus A3
        Channel 3 - A2 Minus A3

        :param parameters [tuple]: Tuple of parameters (channel,
                                                        gain,
                                                        address,
                                                        busnum)
        """
        self.parameters = list(parameters)
        check_params(self.parameters)

        try:
            self.parameters[2]
        except IndexError:
            self.parameters.append(0x48)

        try:
            self.parameters[3]
        except IndexError:
            self.parameters.append(1)

        self.adc = Adafruit_ADS1x15.ADS1115(address=self.parameters[2],
                                            busnum=self.parameters[3])

    def read(self):
        """
        Read a value back from the ADC in the prior-configured channel with the
        prior-configured gain.

        :return [int]: ADC read value
        """
        return self.adc.read_adc_difference(self.parameters[0],
                                            gain=self.parameters[1])

    def change_param(self, position, value):
        """
        Change a parameter in the list of parameters and update the list.

        :param position [int]: Some value [0, 3] to indicate position in list.
        :param value [int|float]: The new value to place in that position.
        """
        assert position in range(4)

        self.parameters[position] = value

        check_params(self.parameters)

        if position >= 2:
            # Rebuild the adc device only if the address or busnum have changed.
            self.adc = Adafruit_ADS1x15.ADS1115(address=self.parameters[2],
                                                busnum=self.parameters[3])


