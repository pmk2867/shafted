#!/usr/bin/python

import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

from shafted import *

# This is currently just in testing stage. This is not to be a representative of
# any final products.

# Configure the ADC to be a differential measurement on channel 0
# (Channel A0 - Channel A1) at a gain of 2/3 (each bit is worth 187.5uV)
# meaning the ADC can detect +/- 6.144V.
my_adc = adc.adc(device_drivers.ADS1115.ads1115_differential((0, 2/3)))

# Configure the two speed counters.
speed_0 = speed.speedCounter()
speed_1 = speed.speedCounter()

# Configure which pins on the Raspberry Pi are active inputs.
GPIO.setup(5, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(6, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# Tie the input pins to the speed counters by making sure the pins
# correctly trigger the speedCounter when the pin falls.
GPIO.add_event_detect(5, GPIO.FALLING, callback=speed_0.trigger)
GPIO.add_event_detect(6, GPIO.FALLING, callback=speed_1.trigger)

start_time = time.time()

prev_time = time.time()
last_print = time.time()
current_adc_val = 0

f = open('test_data.csv', 'w')
f.write('Time,ADC,Speed\n')

while (1):
    if time.time() - prev_time > 0.01:
        current_adc_val = my_adc.read()
        prev_time = time.time()

    if time.time() - last_print >= 0.01:
        f.write('{},{},{}\n'.format(time.time()-start_time,
                                  my_adc.read(),
                                  speed_0.get_hertz()))
        last_print = time.time()

