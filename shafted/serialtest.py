#!/usr/bin/python3

import serial
import spidev

port = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=3.0)
spi = spidev.SpiDev()

spi.open(0, 0)
spi.max_speed_hz = 976000

def send_recv_speed(speed):
    msb = speed >> 8
    lsb = speed & 0xFF
    response = spi.xfer2([msb, lsb])
    inboard = response[0] >> 8 + response[1]
    outboard = response[2] >> 8 + response[3]

    return inboard, outboard

def send_recv_torque(torque):


while True:
    port.write("\r\nSay something:")
    rcv = port.read(10)
    port.write("\r\nYou sent:" + repr(rcv))
