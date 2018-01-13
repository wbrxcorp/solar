#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys,datetime,argparse
import cv2
import database

LOCK_NAME="camera.lock"
LOCK_TIMEOUT="10"

def get_recent_pv_power():
    result = []
    with database.Connection() as cur:
        cur.execute("select hostname,max(piw) as maxpiw from data where t > now() - interval 10 second group by hostname order by maxpiw desc")
        for row in cur:
            result.append({"nodename":row[0],"piw":float(row[1])})
    return result

def capture(device_number, width, height, skip_frames):
    with database.Connection() as cur:
        cur.execute("select get_lock(%s,%s)", (LOCK_NAME,LOCK_TIMEOUT))
        if cur.fetchone()[0] != 1: return None
        #else
        cap = cv2.VideoCapture(device_number)
        cap.set(3, width)
        cap.set(4, height)
        for i in range(skip_frames): r, frame = cap.read()
        return frame

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--device-number", type = int, dest = "device_number", default = 0)
    parser.add_argument("--width", type = int, dest = "width", default=640)
    parser.add_argument("--height", type = int, dest = "height", default=480)
    parser.add_argument("-f", "--skip-frames", type = int, dest = "skip_frames", default=10)
    parser.add_argument("-o", "--output-file", type = str, dest = "output_file", default="camera.jpg")
    parser.add_argument("-q", "--jpeg-quality", type = int, dest = "jpeg_quality", default = 95)
    args = parser.parse_args()

    frame = capture(args.device_number, args.width, args.height, args.skip_frames)
    if frame is None:
        sys.exit(1)

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

    cv2.imwrite(args.output_file, frame, [int(cv2.IMWRITE_JPEG_QUALITY), args.jpeg_quality])
