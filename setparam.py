#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
from pyepsolartracer.client import EPsolarTracerClient

REG_BATTERY_TYPE = "Battery Type"
REG_BATTERY_CAPACITY = "Battery Capacity"

battery_types = {
    1:"1:Sealed",
    2:"2:Gel",
    3:"3:Flooded"
}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--battery-type", help="Battery type(1=Sealed,2=Gel,3=Flooded)", type = int, dest = "battery_type", default = None)
    parser.add_argument("-c", "--battery-capacity", help="Battery capacity in Ah", type = int, dest = "battery_capacity", default = None)
    args = parser.parse_args()

    client = EPsolarTracerClient(port = "/dev/ttyXRUSB0")

    if args.battery_type is None:
        print "Battery type: %s" % (battery_types[client.read_input(REG_BATTERY_TYPE).value])
    else:
        if args.battery_type in (1,2,3):
            client.write_output(REG_BATTERY_TYPE, args.battery_type)
            print "Battery type set to %s" % (battery_types[args.battery_type])
        else:
            print "Invalid battery type"

    if args.battery_capacity is None:
        print "Battery capacity: %dAh" % (client.read_input(REG_BATTERY_CAPACITY).value)
    else:
        client.write_output(REG_BATTERY_CAPACITY, args.battery_capacity)
        print "Battery capacity set to %dAh" % (args.battery_capacity)
