#!/usr/bin/python3

import time
import struct
import serial
import spidev

port = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=0.010)
spi = spidev.SpiDev()

spi.open(0, 0)
spi.max_speed_hz = 976000

def send_recv_speed(speed):
    msb = speed >> 8
    lsb = speed & 0xFF
    response = spi.xfer2([msb, lsb])
    if len(response) != 4:
        return None, None
    else:
        inboard = response[0] >> 8 + response[1]
        outboard = response[2] >> 8 + response[3]

    return inboard, outboard

def send_recv_torque(torque):
    torque = chr(torque)
    if len(torque) < 2:
        torque = torque

    port.write(torque)
    rcv = port.read(4)

    if len(rcv) != 4:
        return None, None
    else:
        rcv = struct.unpack('>I', rcv)[0]
        in_val = rcv >> 16
        out_val = rcv & 0xFFFF

    out_volts = out_val * 0.0001875
    out_force = (9.7847 * out_volts) + 0.0724

    return in_val, out_force

while True:
    time.sleep(0.009)

    in_s, out_s = send_recv_speed(600)
    in_t, out_t = send_recv_torque(20)

    print("    Speed\t|   Torque")
    print("In\tOut\t|In\tOut")
    print(str(in_s) + "\t" + str(out_s) + "\t|" + str(in_t) + "\t" + str(out_t) + "\t")
    print("\n\r")
