#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,threading
import MySQLdb

PORT = 29574

def process_connection(conn, addr):
    print 'Connected by', addr
    sock_as_file = conn.makefile()
    while True:
        data = sock_as_file.readline()
        if data == "": break
        print data.strip()
        conn.send(data)
    conn.close()
    print 'Connection closed by', addr

if __name__ == '__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
    s.bind(("0.0.0.0", PORT))
    s.listen(1)

    try:
        while True:
            conn, addr = s.accept()
            thr = threading.Thread(target=process_connection, args=(conn, addr, ))
            thr.setDaemon(True)
            thr.start()
    except (KeyboardInterrupt, SystemExit):
        s.close()
        print "Exit."
