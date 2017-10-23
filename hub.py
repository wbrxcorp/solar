#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,threading,traceback,datetime,time
import MySQLdb

PORT = 29574

def parse_data(data):
    parsed_data = {}
    for col in data:
        if ':' not in col: continue
        col_splitted = col.split(':', 1)
        parsed_data[col_splitted[0]] = col_splitted[1]
    return parsed_data

def save_data(nodename, data):
    conn = MySQLdb.connect(db="solar")
    conn.autocommit(True)
    cur = conn.cursor()
    now_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    try:
	   cur.execute("replace into data(hostname,t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh) values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)", (nodename,now_str, float(data["piv"]),float(data["pia"]),float(data["piw"]),float(data["bv"]),float(data["poa"]),float(data["load"]),float(data["temp"]),float(data["kwh"]),float(data["lkwh"])))
    except:
        print "# Database error!"
        print traceback.format_exc()
    finally:
        cur.close()
        conn.close()
    return now_str

def process_connection(conn, addr):
    print "# New connection from %s" % addr[0]
    last_time = 0.0
    try:
        nodename = None
        sock_as_file = conn.makefile()
        while True:
            line = sock_as_file.readline()
            if line == "": break
            data_splitted = line.strip().split('\t')
            if data_splitted[0] == "NODATA":
                if nodename is None:
                    print "# NODATA from unknown node"
                else:
                    print "# NODATA from node '%s'" % nodename
            elif data_splitted[0] == "DATA" and nodename is not None:
                data = parse_data(data_splitted[1:])
                piv = float(data["piv"])
                current_time = time.time()
                if piv > 0.0 or current_time >= last_time + 60:
                    now_str = save_data(nodename,data)
                    print "%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f" % (nodename,now_str,piv,float(data["pia"]),float(data["piw"]),float(data["bv"]),float(data["poa"]),float(data["load"]),float(data["temp"]),float(data["kwh"]),float(data["lkwh"]))
                    last_time = current_time
            elif data_splitted[0] == "INIT":
                parsed_data = parse_data(data_splitted[1:])
                if "nodename" in parsed_data:
                    nodename = parsed_data["nodename"]
                    print "# Node name set to '%s'" % nodename

            sys.stdout.flush()
            conn.send("OK\n")
    finally:
        conn.close()
    print "# Connection from %s(%s) closed." % (nodename if nodename is not None else "Unknwon node", addr[0])

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
