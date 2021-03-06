#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,threading,traceback,datetime,time,uuid,numbers
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
        cur.execute("select t,piv,pov from data where hostname=%s order by t desc limit 1", (nodename,))
        last_data_time,last_piv,last_pov = cur.fetchone() or (None, None, None)
        piv = float(data["piv"])
        pov = float(data["bv"])
        piw = float(data["piw"])
        pw = int(data["pw"]) if "pw" in data else None
        pw1 = int(data["pw1"]) if "pw1" in data else None
        load = float(data["load"]) if "load" in data else None
        soc = int(data["soc"]) if "soc" in data else None
        aiw = float(data["aiw"]) if "aiw" in data else None
        aiv = float(data["aiv"]) if "aiv" in data else None
        aia = float(data["aia"]) if "aia" in data else None
        gpio = int(data["gpio"]) if "gpio" in data else None
        cs = int(data["cs"]) if "cs" in data else None
        if last_data_time is None or datetime.datetime.now() - last_data_time >= datetime.timedelta(minutes=1) or last_piv > 0.0 or piv > 0.0 or pov != last_pov:
            cur.execute("replace into data(hostname,t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh,soc,aiv,aia,aiw) values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)", (nodename,now_str, piv,float(data["pia"]),piw,pov,float(data["poa"]),load,float(data["temp"]),float(data["kwh"]),float(data["lkwh"]),soc,aiv,aia,aiw))
            saved = True

        if soc == 0: response_data["sleep"] = 255
        elif piw < 0.5 and (load is None or load < 0.01):
            response_data["sleep"] = 60

        bv_compensation = float(data["btcv"]) if "btcv" in data else 0.0

        cur.execute("select avg(pov) as bv,avg(pov)-%s as cbv,avg(piw) as piw,avg(loadw) as loadw,avg(temp) as temp,avg(aiw) as aiw from data where hostname=%s and t > now() - interval 1 minute", (bv_compensation, nodename, ))
        row = cur.fetchone()
        if row is not None:
            bv,compensated_bv,piw,loadw,temp,aiw = row
            cur.execute("select `key`,int_value from bv_conditions where nodename=%s and (gt is null or gt < %s) and (lt is null or lt > %s)", (nodename, compensated_bv, bv))
            for row in cur:
                key,int_value = row
                if key in data and int_value != int(data[key]):
                    response_data[key] = int_value

            cur.execute("select `key`,expression from conditions where nodename=%s", (nodename, ))
            for row in cur:
                key,expression = row
                try:
                    value = eval(expression, {}, {"pw":pw,"pw1":pw1,"piw":piw,"bv":compensated_bv,"pov":compensated_bv,"loadw":loadw,"temp":temp,"soc":soc,"aiw":aiw,"gpio":gpio,"cs":cs})
                    if isinstance(value,numbers.Number):
                        response_data[key] = value
                        print "#%s: set %s to %d" % (nodename, key, value)
                except:
                    print "Error evaluating xpression '%s'." % expression

        cur.execute("select id,`key`,int_value from schedule where nodename=%s and (t is null or t <= now())", (nodename,))
        for row in cur:
            id,key,int_value = row
            response_data[key] = int_value
            cur.execute("delete from schedule where id=%s", (id,))

    return (now_str, response_data, saved)

def get_node_config(nodename):
    with database.Connection() as cur:
        cur.execute("select battery_type,battery_capacity,gpio from nodes where nodename=%s", (nodename,))
        row = cur.fetchone()
        if row is None: return None
        # else
        battery_type, battery_capacity, gpio = row
        # battery_type : Battery type(1=Sealed,2=Gel,3=Flooded,4=3S/6S,5=7S)
        # battery_capacity : Battery capacity in Ah
        config = {"bt":int(battery_type), "bc":int(battery_capacity)}
        if gpio is not None: config["gpio"] = 1 if gpio else 0
        return config

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
                        if "cs" not in data: data["cs"] = 0
                        if "btcv" not in data: data["btcv"] = 0.0
                        if "rssi" not in data: data["rssi"] = 0
                        if "soc" not in data: data["soc"] = 0
                        if "aiw" not in data: data["aiw"] = 0
                        if "gpio" not in data: data["gpio"] = 0
                        print "%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t#%d\t%d\t%.2f\t%d\t%d\t%d\t%.2f\t%d" % (nodename,now_str,piv,float(data["pia"]),float(data["piw"]),float(data["bv"]),float(data["poa"]),float(data["load"]),float(data["temp"]),float(data["kwh"]),float(data["lkwh"]),int(data["pw"]),int(data["pw1"]),float(data["btcv"]),int(data["cs"]),int(data["rssi"]),float(data["soc"]),float(data["aiw"]),int(data["gpio"]))
            elif data_splitted[0] == "INIT":
                parsed_data = parse_data(data_splitted[1:])
                if "nodename" in parsed_data:
                    nodename = parsed_data["nodename"]
                    print "# Node name is '%s'" % nodename
                    # send controller params
                    session_id = uuid.uuid4().hex[:8]
                    response_data["session"] = session_id
                    now = datetime.datetime.now()
                    d = now.strftime("%Y%m%d")
                    t = now.strftime("%H%M%S")
                    response_data["d"] = d
                    response_data["t"] = t
                    node_config = get_node_config(nodename)
                    if node_config is not None:
                        for k,v in node_config.iteritems():
                            if v is not None: response_data[k] = v
                if "boot" in parsed_data:
                    print "# it just booted"

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
