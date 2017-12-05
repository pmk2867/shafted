#!/usr/bin/python

import time

class speedCounter(object):
    """
    Implements an object that keeps track of the per-second frequency of
    something that gets triggered externally, and can convert to per-minute.
    """

    def __init__(self):
        self.difference = 0
        self.timeNow = time.time()
        self.timeLast = time.time()

    def trigger(self, channel=None):
        """
        Checks current time when the trigger occurs and finds the difference
        from the last trigger.

        :param channel [int]: Optional parameter indicating triggered channel.
        """
        self.timeLast = self.timeNow
        self.timeNow = time.time()

        self.difference = self.timeNow - self.timeLast

    def get_hertz(self):
        """
        Returns the frequency of the triggered event, based on the most recent
        trigger. Output is rounded to the third decimal.

        :return frequency [Hz]
        """
        if self.difference != 0:
            return round(1/self.difference, 3)
        else:
            return False

    def get_per_minute(self):
        """
        Returns the frequency of the triggered event, based on the most recent
        trigger in units of pulses per minute. Output is rounded to the third
        decimal.

        :return frequency [1/minute]
        """
        if self.difference != 0:
            return round(1/self.difference * 60, 3)
        else:
            return False

    def get_per_hour(self):
        """
        Returns the frequency of the triggered event, based on the most recent
        trigger in units of pulses per hour. Output is rounded to the third
        decimal.

        :return frequency [1/hour]
        """
        if self.difference != 0:
            return round(1/self.difference * 60 * 60, 3)
        else:
            return False
