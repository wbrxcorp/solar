#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys,io,datetime,argparse
import matplotlib.pyplot,matplotlib.dates
import MySQLdb

def load_data(hostname, date_str = None):
    conn = MySQLdb.connect(db="solar")
    cur = conn.cursor()
    try:
        if date_str is None:
            cur.execute("select convert(current_date(), datetime), convert(current_date(), datetime) + interval 1 day")
        else:
            cur.execute("select convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime), convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime) + interval 1 day", (date_str,date_str))

        starttime, endtime = cur.fetchone()

        cur.execute("show variables like 'version'")
        if "MariaDB" in cur.fetchone()[1]:
            cur.execute("create temporary table data1(t datetime primary key,t5 datetime unique,piv float,pia float,piw float,pov float,poa float,loadw float,temp float,kwh float,lkwh float) engine memory")
            cur.execute("insert into data1(t,t5,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh) select t,t - interval 5 minute,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh from data where data.t >= %s and data.t < %s", (starttime, endtime))

            cur.execute("select data1.t,data1.piv,avg(data2.piv),data1.pia,avg(data2.pia),data1.piw,avg(data2.piw),data1.pov,avg(data2.pov),data1.poa,avg(data2.poa),data1.loadw,avg(data2.loadw),data1.temp,data1.kwh,data1.lkwh from data1,data1 as data2 where data2.t between data1.t5 and data1.t group by data1.t order by data1.t")
        else:
            cur.execute("select data1.t,data1.piv,avg(data2.piv),data1.pia,avg(data2.pia),data1.piw,avg(data2.piw),data1.pov,avg(data2.pov),data1.poa,avg(data2.poa),data1.loadw,avg(data2.loadw),data1.temp,data1.kwh,data1.lkwh from data as data1,data as data2 where data1.t >= %s and data1.t < %s and data2.t between data1.t - interval 5 minute and data1.t group by data1.t order by data1.t", (starttime,  endtime))

        return (starttime, endtime, [(row[0],row[1:]) for row in cur])
    finally:
        cur.close()
        conn.close()

def generate_graph(hostname, date_str = None, pov_ymin = 10.5, pov_ymax = 15.0):
    (starttime, endtime, data) = load_data(hostname, date_str)

    matplotlib.rc("font", family="VL PGothic")

    time_range=3
    x = [row[0] for row in data]
    xfmt = matplotlib.dates.DateFormatter("%H")
    xloc = matplotlib.dates.HourLocator()

    fig, (piw,bv,kwh) = matplotlib.pyplot.subplots(ncols=1, nrows=3,figsize=(12,10), sharex=True)

    fig.suptitle(starttime.strftime('%Y-%m-%d (%a)'))
    #fig.tight_layout()

    piw.set_ylabel(u"電力(W)")
    piw.xaxis.set_major_locator(xloc)
    piw.xaxis.set_major_formatter(xfmt)
    piw.set_xlim(starttime, endtime)
    #piw.set_ylim(0, 150)
    #piw.tick_params(labelsize=8)
    piw.grid(True)
    piw.plot(x, [row[1][4] for row in data], label=u"発電(5秒間隔)", linewidth=0.5)
    piw.plot(x, [row[1][5] for row in data], label=u"発電(5分平均)", linewidth=2,color="g")
    piw.plot(x, [row[1][11] for row in data], label=u"消費(5分平均)", linewidth=2,color="r")
    piw.legend()

    bv.set_ylabel(u"バッテリー電圧(V)")
    bv.set_ylim(pov_ymin, pov_ymax)
    #bv.tick_params(labelsize=8)
    bv.grid(True)
    bv.axhline(12.0, linestyle="--", color="g")
    bv.plot(x, [row[1][6] for row in data], label=u"5秒間隔", linewidth=0.5)
    bv.plot(x, [row[1][7] for row in data], label=u"5分平均", linewidth=2,color="r")
    bv.legend()

    kwh.set_ylabel(u"当日の電力量(kWh)")
    #kwh.set_ylim(0.0, 1.0)
    #kwh.tick_params(labelsize=8)
    kwh.grid(True)
    kwh.plot(x, [row[1][13] for row in data], label=u"発電", color="g")
    kwh.plot(x, [row[1][14] for row in data], label=u"消費", color="r")
    kwh.legend()

    buf = io.BytesIO()
    fig.savefig(buf, format="png")
    buf.seek(0)
    return buf.read()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--date", type = str, dest = "date", default = None)
    parser.add_argument("-o", "--output", type = str, dest = "output", default="graph.png")
    args = parser.parse_args()

    hostname = "motion"
    graph = generate_graph(hostname, args.date)
    with open(args.output, "w") as f:
        f.write(graph)
