#!/usr/bin/python
import time,datetime,sys
import MySQLdb
from pyepsolartracer.client import EPsolarTracerClient
from pyepsolartracer.registers import registers,coils

registers = [
    "Charging equipment input voltage",
    "Charging equipment input current",
    "Charging equipment input power",
    "Charging equipment output voltage",
    "Charging equipment output current",
    "Discharging equipment output voltage",
    "Battery Temperature",
    "Generated energy today",
    "Discharging equipment output power"
]

client = EPsolarTracerClient(port = "/dev/ttyXRUSB0")
client.connect()

response = client.read_device_info()
print "# Manufacturer:", repr(response.information[0])
print "# Model:", repr(response.information[1])
print "# Version:", repr(response.information[2])

conn = MySQLdb.connect(db="solar")
conn.autocommit(True)

cur = conn.cursor()

try:
    while True:
        values = [ client.read_input(x).value for x in registers ]
        now_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    	cur.execute("replace into data(t,piv,pia,piw,pov,poa,bv,temp,kwh) values(%s,%s,%s,%s,%s,%s,%s,%s,%s)", (now_str, values[0],values[1],values[2],values[3],values[4],values[5],values[6],values[7]))

        print "%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f" % (now_str, values[0],values[1],values[2],values[3],values[4],values[5],values[6],values[7],values[8])
        sys.stdout.flush()
        time.sleep(5)
finally:
    cur.close()
    conn.close()
    client.close()
    print "# End of file"
