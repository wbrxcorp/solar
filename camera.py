#!/usr/bin/env python
# -*- coding: utf-8 -*-

import datetime,argparse
import cv2
import database

def get_recent_pv_power():
    result = []
    with database.Connection() as cur:
        cur.execute("select hostname,max(piw) as maxpiw from data where t > now() - interval 10 second group by hostname order by maxpiw desc")
        for row in cur:
            result.append({"nodename":row[0],"piw":float(row[1])})
    return result

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--device-number", type = int, dest = "device_number", default = 0)
    parser.add_argument("--width", type = int, dest = "width", default=640)
    parser.add_argument("--height", type = int, dest = "height", default=480)
    parser.add_argument("-f", "--skip-frames", type = int, dest = "skip_frames", default=10)
    parser.add_argument("-o", "--output-file", type = str, dest = "output_file", default="camera.jpg")
    args = parser.parse_args()

    cap = cv2.VideoCapture(args.device_number)
    cap.set(3, args.width)
    cap.set(4, args.height)
    for i in range(args.skip_frames): r, frame = cap.read()

    text = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    fontface = cv2.FONT_HERSHEY_PLAIN
    fontscale = 2
    thickness = 2
    textsize, _ = cv2.getTextSize(text, fontface, fontscale, thickness)
    cv2.putText(frame,text,(args.width - textsize[0],args.height),fontface,fontscale,(255,255,255),thickness)

    recent_pv_power = get_recent_pv_power()
    text_pos = args.height
    for node in reversed(recent_pv_power):
        text = "%s: %.2fW" % (node["nodename"],node["piw"])
        textsize, baseline = cv2.getTextSize(text, fontface, fontscale, thickness)
        cv2.putText(frame,text,(0, text_pos),fontface,fontscale,(255,255,255),thickness)
        text_pos -= textsize[1] + baseline

    cv2.imwrite(args.output_file, frame)

