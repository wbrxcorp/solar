#!/usr/bin/python2

import time
import paho.mqtt.client as mqtt
import database

host = "localhost"

def on_connect(client, userdata, flags, response_code):
  print "MQTT connected."

def do_gearchange_if_necessary(client):
  with database.Connection() as cur:
    cur.execute("select hostname,avg(piw) as piw,avg(pov) as pov,avg(loadw) as loadw,avg(temp) as temp from data where t > now() - interval 1 minute group by hostname")
    for row in cur:
      nodename,piw,pov,loadw,temp = row
      if (pov > 14.4 or (piw > loadw and pov > 14.0)) and temp < 40.0: client.publish("gearchange-%s" % nodename, "up")
      elif pov < 13.0 or temp > 42.0: client.publish("gearchange-%s" % nodename, "down")

if __name__ == '__main__':
  client = mqtt.Client(protocol=mqtt.MQTTv311)
  client.on_connect = on_connect
  client.connect(host, keepalive=60)

  client.loop()
  client.loop_start()

  while True:
    do_gearchange_if_necessary(client)
    time.sleep(60)
