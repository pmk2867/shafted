#!/usr/bin/python3

import argparse
import datetime
from influxdb import InfluxDBClient
import json
import serial
import RPi.GPIO as GPIO
import signal
import spidev
import struct
import time

control = 6

GPIO.setmode(GPIO.BCM)
GPIO.setup(control, GPIO.OUT)
GPIO.output(control, GPIO.LOW)

torque_dev = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=0.010)
speed_dev = serial.Serial("/dev/ttyUSB0", baudrate=115200, timeout=0.010)

units = {'torque': 'Nm', 'speed': 'RPM', 'position': 'Degrees'}

class Messenger:
    def __init__(self, db_client):
        self.client = db_client
        self.messages = []
        self.last_update = datetime.datetime.now()

    def add_message(self, name, side, value, timestamp):
        message = {}

        assert name in ['speed', 'torque', 'position']
        assert side in ['inboard', 'outboard']

        message['measurement'] = name
        message['tags'] = {'side' : side}
        message['time'] = timestamp
        message['fields'] = {units[name] : value}

        self.messages.append(message)

    def send_messages(self):
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
    device_addr = 255

    speed_dev.write(struct.pack('>B', device_addr))
    speed_dev.write(struct.pack('>B', speed))

    rcv = speed_dev.read(12)
    if len(rcv) != 12:
        return 0, 0, 0, 0, 0, 0
    else:
        rcv = struct.unpack('>12B', rcv)
        print("Speed =", rcv)

        speed1 = (rcv[0] << 8) + rcv[1]
        speed2 = (rcv[2] << 8) + rcv[3]
        pos1 =   (rcv[4] << 8) + rcv[5]

        speed3 = (rcv[6] << 8) + rcv[7]
        speed4 = (rcv[8] << 8) + rcv[9]
        pos2 =   (rcv[10] << 8) + rcv[11]

        return speed1, speed2, pos1, speed3, speed4, pos2

def send_recv_torque(torque):
    device_addr = 254

    torque_dev.write(struct.pack('>B', device_addr))
    torque_dev.write(struct.pack('>B', torque))
    rcv = torque_dev.read(4)

    if len(rcv) != 4:
        return 0, 0
    else:
        rcv = struct.unpack('>I', rcv)[0]
        in_val = rcv >> 16
        out_val = rcv & 0xFFFF

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
        for i in range(200):
            time.sleep(0.005)
            in1_s, in2_s, in_p, out1_s, out2_s, out_p = send_recv_speed(i)
            time.sleep(0.005)
            in_t, out_t = send_recv_torque(i)

            if __debug__:
                print("    Speed\t\t\t|   Torque\t|    Position")
                print("In1\tIn2\tOut1\tOut2\t|In\tOut\t|In\tOut\t")
                print(str(in1_s) + "\t" +
                      str(in2_s) + "\t" +
                      str(out1_s) + "\t" +
                      str(out2_s) + "\t|" +
                      str(in_t) + "\t" +
                      str(out_t) + "\t|" +
                      str(in_p) + "\t" +
                      str(out_p) + "\t")
                print("\n\r")

            timestamp = int(time.time() * 1000000000)

            database.add_message('torque', 'inboard', in_t, timestamp)
            database.add_message('torque', 'outboard', out_t, timestamp)
            database.add_message('speed', 'inboard', in1_s, timestamp)
            database.add_message('speed', 'outboard', out1_s, timestamp)
            database.add_message('position', 'inboard', in_p, timestamp)
            database.add_message('position', 'outboard', out_p, timestamp)

            database.send_messages()
if __name__ == "__main__":
    main()
    sys.exit(0)
