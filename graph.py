#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys,io,datetime
import matplotlib.pyplot,matplotlib.dates
import MySQLdb

def load_data(date_str = None):
    conn = MySQLdb.connect(db="solar")
    cur = conn.cursor()
    try:
        if date_str is None:
            cur.execute("select convert(current_date(), datetime), convert(current_date(), datetime) + interval 1 day")
        else:
            cur.execute("select convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime), convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime) + interval 1 day", (date_str,date_str))

        starttime, endtime = cur.fetchone()

        cur.execute("select data1.t,data1.piv,avg(data2.piv),data1.pia,avg(data2.pia),data1.piw,avg(data2.piw),data1.pov,avg(data2.pov),data1.poa,avg(data2.poa),data1.bv,avg(data2.bv),data1.temp,data1.kwh from data as data1,data as data2 where data1.t >= %s and data1.t < %s and data2.t between data1.t - interval 5 minute and data1.t group by data1.t order by data1.t", (starttime,  endtime))
        return (starttime, endtime, [(row[0],row[1:]) for row in cur])
    finally:
        cur.close()
        conn.close()

def generate_graph(date_str = None):
    (starttime, endtime, data) = load_data(date_str)

    matplotlib.rc("font", family="VL PGothic")

    time_range=3
    x = [row[0] for row in data]
    xfmt = matplotlib.dates.DateFormatter("%H")
    xloc = matplotlib.dates.HourLocator()

    fig, (piw,bv,kwh) = matplotlib.pyplot.subplots(ncols=1, nrows=3,figsize=(12,10), sharex=True)

    fig.suptitle(starttime.strftime('%Y-%m-%d (%a)'))
    #fig.tight_layout()

    piw.set_ylabel(u"パネル入力(W)")
    piw.xaxis.set_major_locator(xloc)
    piw.xaxis.set_major_formatter(xfmt)
    piw.set_xlim(starttime, endtime)
    #piw.set_ylim(0, 150)
    #piw.tick_params(labelsize=8)
    piw.grid(True)
    piw.plot(x, [row[1][4] for row in data], linewidth=0.5)
    piw.plot(x, [row[1][5] for row in data], linewidth=2)

    bv.set_ylabel(u"バッテリー電圧(V)")
    #bv.set_ylim(10, 15)
    #bv.tick_params(labelsize=8)
    bv.grid(True)
    bv.plot(x, [row[1][10] for row in data], linewidth=0.5)
    bv.plot(x, [row[1][11] for row in data], linewidth=2)

    kwh.set_ylabel(u"当日発電した電力量(kWh)")
    #kwh.set_ylim(0.0, 1.0)
    #kwh.tick_params(labelsize=8)
    kwh.grid(True)
    kwh.plot(x, [row[1][13] for row in data])

    buf = io.BytesIO()
    fig.savefig(buf, format="png")
    buf.seek(0)
    return buf.read()

if __name__ == '__main__':
    graph = generate_graph(sys.argv[1] if len(sys.argv) > 1 else None)
    with open("graph.png", "w") as f:
        f.write(graph)
