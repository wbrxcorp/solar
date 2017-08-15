#!/usr/bin/python
import time,datetime,sys
from pyepsolartracer.client import EPsolarTracerClient
from pyepsolartracer.registers import registers,coils

client = EPsolarTracerClient(port = "/dev/ttyXRUSB0")
client.connect()

response = client.read_device_info()
print "# Manufacturer:", repr(response.information[0])
print "# Model:", repr(response.information[1])
print "# Version:", repr(response.information[2])

registers = [
    "Charging equipment input voltage",
    "Charging equipment input current",
    "Charging equipment input power",
    "Charging equipment output voltage",
    "Charging equipment output current",
    "Discharging equipment output voltage",
    "Battery Temperature",
    "Generated energy today"
]

try:
    while True:
        values = [ client.read_input(x).value for x in registers ]
        print "%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f" % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"), values[0],values[1],values[2],values[3],values[4],values[5],values[6],values[7])
        sys.stdout.flush()
        time.sleep(5)
finally:
    client.close()
    print "# End of file"
