#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,threading,traceback,datetime,time,uuid
import database

PORT = 29574

def parse_data(data):
    parsed_data = {}
    for col in data:
        if ':' not in col: continue
        col_splitted = col.split(':', 1)
        parsed_data[col_splitted[0]] = col_splitted[1]
    return parsed_data

def process_data(nodename, data):
    now_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    response_data = {}
    saved = False

    with database.Connection() as cur:
        cur.execute("select max(t) from data where hostname=%s", (nodename,))
        last_data_time = cur.fetchone()[0]
        piv = float(data["piv"])
        bv = float(data["bv"])
        if last_data_time is None or datetime.datetime.now() - last_data_time >= datetime.timedelta(minutes=1) or piv > 0.0:
            cur.execute("replace into data(hostname,t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh) values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)", (nodename,now_str, piv,float(data["pia"]),float(data["piw"]),bv,float(data["poa"]),float(data["load"]),float(data["temp"]),float(data["kwh"]),float(data["lkwh"])))
            saved = True

        cur.execute("select `key`,int_value from bv_conditions where nodename=%s and (gt is null or gt < %s) and (lt is null or lt > %s)", (nodename, bv, bv))
        for row in cur:
            key,int_value = row
            if key in data and int_value != int(data[key]):
                response_data[key] = int_value

        cur.execute("select id,`key`,int_value from schedule where nodename=%s and (t is null or t <= now())", (nodename,))
        for row in cur:
            id,key,int_value = row
            response_data[key] = int_value
            cur.execute("delete from schedule where id=%s", (id,))

    return (now_str, response_data, saved)

def get_node_config(nodename):
    with database.Connection() as cur:
        cur.execute("select battery_type,battery_capacity from nodes where nodename=%s", (nodename,))
        row = cur.fetchone()
        if row is None: return None
        # else
        battery_type, battery_capacity = row
        # battery_type : Battery type(1=Sealed,2=Gel,3=Flooded)
        # battery_capacity : Battery capacity in Ah
        return {"bt":int(battery_type), "bc":int(battery_capacity)}

def process_connection(conn, addr):
    print "# New connection from %s" % addr[0]
    last_time = 0.0
    try:
        sock_as_file = conn.makefile()
        while True:
            line = sock_as_file.readline()
            if line == "": break
            data_splitted = line.strip().split('\t')
            response_data = {}
            if data_splitted[0] == "NODATA":
                parsed_data = parse_data(data_splitted[1:])
                if "nodename" in parsed_data:
                    print "# NODATA from node '%s'" % parsed_data["nodename"]
                else:
                    print "# NODATA from unknown node"
            elif data_splitted[0] == "DATA":
                data = parse_data(data_splitted[1:])
                if "nodename" in data:
                    nodename = data["nodename"]
                    piv = float(data["piv"])

                    now_str,_response_data,saved = process_data(nodename,data)
                    for k,v in _response_data.iteritems():
                        if v is not None: response_data[k] = v
                    if saved:
                        if "pw" not in data: data["pw"] = 0
                        if "pw1" not in data: data["pw1"] = 0
                        print "%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t#%d\t%d" % (nodename,now_str,piv,float(data["pia"]),float(data["piw"]),float(data["bv"]),float(data["poa"]),float(data["load"]),float(data["temp"]),float(data["kwh"]),float(data["lkwh"]),int(data["pw"]),int(data["pw1"]))
            elif data_splitted[0] == "INIT":
                parsed_data = parse_data(data_splitted[1:])
                if "nodename" in parsed_data:
                    nodename = parsed_data["nodename"]
                    print "# Node name is'%s'" % nodename
                    # send controller params
                    session_id = uuid.uuid4().hex[:8]
                    response_data["session"] = session_id
                    now = datetime.datetime.now()
                    d = now.strftime("%Y%m%d")
                    t = now.strftime("%H%M%S")
                    response_data["d"] = d
                    response_data["t"] = t
                    response_data["pw"] = 1
                    node_config = get_node_config(nodename)
                    if node_config is not None:
                        for k,v in node_config.iteritems():
                            if v is not None: response_data[k] = v

            sys.stdout.flush()

            response = "OK"
            for k,v in response_data.iteritems():
                if v is not None: response += "\t%s:%s" % (k, str(v))
            conn.send(response + "\n")
    finally:
        conn.close()
    print "# Connection from %s closed." % (addr[0],)

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
