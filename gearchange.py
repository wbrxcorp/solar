#!/usr/bin/python2

import time
import paho.mqtt.client as mqtt
import database

host = "localhost"
force_check_topic = "gearchange-force-check"

def on_connect(client, userdata, flags, response_code):
    client.subscribe(force_check_topic)
    print "MQTT connected."

def on_message(client, userdata, msg):
    if msg.topic != force_check_topic: return
    #else
    print "Force check request received."
    do_gearchange_if_necessary(client)

def do_gearchange_if_necessary(client):
    with database.Connection() as cur:
        cur.execute("select hostname,avg(piw) as piw,avg(pov) as pov,avg(loadw) as loadw,avg(temp) as temp,avg(soc) as soc from data where t > now() - interval 1 minute group by hostname")
        for row in cur:
            nodename,piw,pov,loadw,temp,soc = row
            topic = "gearchange-%s" % nodename
            if (pov > 14.4 or (piw > loadw and (pov > 14.0 or soc == 100))) and temp < 40.0: client.publish(topic, "up")
            elif pov < 13.0 or temp > 42.0: client.publish(topic, "down")
            else: client.publish(topic, "stay")

if __name__ == '__main__':
    client = mqtt.Client(protocol=mqtt.MQTTv311)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(host, keepalive=60)

    client.loop()
    client.loop_start()

    while True:
        do_gearchange_if_necessary(client)
        time.sleep(60)
