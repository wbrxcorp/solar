#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import cv2

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
    cv2.imwrite(args.output_file, frame)
