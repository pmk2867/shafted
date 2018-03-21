#!/usr/bin/python3

import argparse
import datetime
from influxdb import InfluxDBClient
import json
import serial
import signal
import spidev
import struct
import time

torque_dev = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=0.010)
speed_dev = spidev.SpiDev()

speed_dev.open(0, 0)
speed_dev.max_speed_hz = 976000

units = {'torque': 'Nm', 'speed': 'RPM'}

class Messenger:
    def __init__(self, db_client):
        self.client = db_client
        self.messages = []
        self.last_update = datetime.datetime.now()

    def add_message(self, name, side, value, timestamp):
        message = {}

        assert name in ['speed', 'torque']
        assert side in ['inboard', 'outboard']

        message['measurement'] = name
        message['tags'] = {'side' : side}
        message['time'] = timestamp
        message['fields'] = {units[name] : value}

        if __debug__:
            print (message)
        self.messages.append(message)

    def send_messages(self):
        '''
        length = len(self.messages)
        if length == 0:
            return
        current_time = datetime.datetime.now()
        if length < high_water_mark and \
            (current_time - self.last_update) < datetime.timedelta(seconds=1):
            return
        if __debug__:
            print("Flushing {} messages".format(length))
        '''
        self.client.write_points(self.messages, protocol='json')
        self.last_update = datetime.datetime.now()
        self.messages = []

def establish_database_connection(host, port, user, password, database):
    client = InfluxDBClient(host, port, user, password, database)

    return client

def sig_handler(_signo, _stack_frame):
    print ("Exiting script...")
    sys.exit(0)

def send_recv_speed(speed):
    msb = speed >> 8
    lsb = speed & 0xFF
    response = speed_dev.xfer2([msb, lsb])
    if len(response) != 4:
        return 0, 0
    else:
        inboard = response[0] >> 8 + response[1]
        outboard = response[2] >> 8 + response[3]

    return inboard, outboard

def send_recv_torque(torque):
    torque_dev.write(struct.pack('>2B', torque, 0))
    rcv = torque_dev.read(4)

    if len(rcv) != 4:
        return 0, 0
    else:
        rcv = struct.unpack('>I', rcv)[0]
        in_val = rcv >> 16
        out_val = rcv & 0xFFFF
        inboard = (rcv >> 16 & 0xFF00) + ((rcv >> 16) & 0xFF)
        outboard = ((rcv & 0xFF00) >> 8) + (rcv & 0xFF)

    return in_val, out_val

def check_valid_port(value):
        try:
            ivalue = int(value)
        except ValueError:
            raise argparse.ArgumentTypeError("{} is an invalid integer.".format(value))
        if ivalue < 1 or ivalue > 65535:
            raise argparse.ArgumentTypeError("{} is a port outside the valid range [1, 65535].".format(ivalue))
        return ivalue


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("--influxdb_host",
                        type=str,
                        help="InfluxDB Host, defaults to localhost",
                        default="localhost")
    parser.add_argument("--influxdb_port",
                        type=check_valid_port,
                        help="InfluxDB port to use for connections. Defaults to 8086",
                        default=8086)
    parser.add_argument("--influxdb_user",
                        type=str,
                        help="InfluxDB User. Defaults to 'admin'",
                        default="admin")
    parser.add_argument("--influxdb_pass",
                        type=str,
                        help="InfluxDB password. Defaults to 'admin'",
                        default="admin")
    parser.add_argument("--influxdb_database",
                        type=str,
                        help="InfluxDB database to use for storing data.",
                        required=True)
    return parser.parse_args()

def main():
    args = parse_args()

    signal.signal(signal.SIGTERM, sig_handler)

    db_client = establish_database_connection(args.influxdb_host,
                                              args.influxdb_port,
                                              args.influxdb_user,
                                              args.influxdb_pass,
                                              args.influxdb_database)

    database = Messenger(db_client)

    while True:
        time.sleep(0.009)

        in_s, out_s = send_recv_speed(600)
        in_t, out_t = send_recv_torque(20)

        if __debug__:
            print("    Speed\t|   Torque")
            print("In\tOut\t|In\tOut")
            print(str(in_s) + "\t" + str(out_s) + "\t|" + str(in_t) + "\t" + str(out_t) + "\t")
            print("\n\r")

        timestamp = int(time.time() * 1000000000)

        database.add_message('torque', 'inboard', in_t, timestamp)
        database.add_message('torque', 'outboard', out_t, timestamp)
        database.add_message('speed', 'inboard', in_s, timestamp)
        database.add_message('speed', 'outboard', out_s, timestamp)

        database.send_messages()

if __name__ == "__main__":
    main()
    sys.exit(0)
