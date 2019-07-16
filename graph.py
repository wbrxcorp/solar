#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys,os,io,datetime,argparse,socket
import matplotlib.pyplot,matplotlib.dates

import database

if os.environ.get('DISPLAY','') == '':
    matplotlib.use('Agg')

def load_data(hostname, date_str = None):
    with database.Connection() as cur:
        if date_str is None:
            cur.execute("select convert(current_date(), datetime), convert(current_date(), datetime) + interval 1 day")
        else:
            cur.execute("select convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime), convert(str_to_date(%s,'%%Y-%%m-%%d'), datetime) + interval 1 day", (date_str,date_str))

        starttime, endtime = cur.fetchone()

        cur.execute("create temporary table data1(t datetime,piv float,pia float,piw float,pov float,poa float,loadw float,temp float,kwh float,lkwh float,soc float,aiw float) engine memory")
        cur.execute("create temporary table data2(t datetime,piv float,pia float,piw float,pov float,poa float,loadw float,temp float) engine memory")
        cur.execute("insert into data1(t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh,soc,aiw) select t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh,soc,aiw from data where hostname = %s and data.t >= %s and data.t < %s", (hostname, starttime, endtime))
        cur.execute("insert into data2(t,piv,pia,piw,pov,poa,loadw,temp) select t,piv,pia,piw,pov,poa,loadw,temp from data where hostname = %s and data.t >= %s - interval 5 minute and data.t < %s", (hostname, starttime, endtime))
        cur.execute("create index idx_t on data2(t) using btree")

        cur.execute("select data1.t,data1.piv,avg(data2.piv),data1.pia,avg(data2.pia),data1.piw,avg(data2.piw),data1.pov,avg(data2.pov),data1.poa,avg(data2.poa),data1.loadw,avg(data2.loadw),data1.temp,avg(data2.temp),kwh,lkwh,soc,aiw from data1,data2 where data2.t between data1.t - interval 5 minute and data1.t group by data1.t,data1.piv,data1.pia,data1.piw,data1.pov,data1.poa,data1.loadw,data1.temp,kwh,lkwh,soc,aiw order by data1.t")

        return (starttime, endtime, [(row[0],row[1:]) for row in cur])

def generate_graph(hostname, date_str = None, pov_ymin = None, pov_ymax = None, pov_middle = None, pov_low = None):
    (starttime, endtime, data) = load_data(hostname, date_str)
    if len(data) == 0:
        print "No data."
        return None

    matplotlib.rc("font", family="VL PGothic")

    time_range=3
    x = [row[0] for row in data]
    xfmt = matplotlib.dates.DateFormatter("%H")
    xloc = matplotlib.dates.HourLocator()

    fig, (piw,bv,kwh,temp) = matplotlib.pyplot.subplots(ncols=1, nrows=4,figsize=(12,12), sharex=True)

    fig.suptitle("Hostname: '%s'\nDate:%s" % (hostname, starttime.strftime('%Y-%m-%d (%a)')))
    #fig.tight_layout()

    hrs = list(data[0][0].replace(hour=hr,minute=0,second=0) for hr in [0,6,12,18])
    hrs.append(hrs[0] + datetime.timedelta(days=1))

    for sp in [piw,bv,kwh,temp]:
        sp.axvline(hrs[2],linestyle="-", color="grey")
        sp.axvspan(hrs[0],hrs[1],facecolor="lightgrey", edgecolor="none")
        sp.axvspan(hrs[3],hrs[4],facecolor="lightgrey", edgecolor="none")

    piw.set_ylabel(u"電力(W)")
    piw.xaxis.set_major_locator(xloc)
    piw.xaxis.set_major_formatter(xfmt)
    piw.set_xlim(starttime, endtime)

    #piw.set_ylim(0, 150)
    #piw.tick_params(labelsize=8)
    piw.grid(True)
    piw.plot(x, [row[1][4] for row in data], label=u"発電(5秒間隔)", linewidth=0.5,zorder=1)
    piw.plot(x, [row[1][5] for row in data], label=u"発電(5分平均)", linewidth=2,color="g",zorder=3)
    piw.plot(x, [row[1][11] for row in data], label=u"消費(5分平均)", linewidth=2,color="r",zorder=4)

    aiw = [row[1][17] for row in data]
    aiw_present = False
    for i in range(0, len(aiw)):
        if aiw[i] == None:
            aiw[i] = 0.0
        else:
            aiw_present = True

    if aiw_present: piw.step(x, aiw, label=u"AUX(5秒間隔)", color="y",linewidth=0.5,alpha=0.5,zorder=2)
    piw.legend()

    if pov_ymin is None: pov_ymin = 10.5
    if pov_ymax is None: pov_ymax = 15.0
    if pov_middle is None: pov_middle = 12.0
    if pov_low is None: pov_low = 11.1

    pov_multiplier = 2 if data[-1][1][6] > 18.0 else 1
    pov_ymin *= pov_multiplier
    pov_ymax *=	pov_multiplier

    bv.set_ylabel(u"バッテリー電圧(V)")
    bv.set_ylim(pov_ymin, pov_ymax)
    #bv.tick_params(labelsize=8)
    bv.grid(True)
    bv.axhline(pov_middle * pov_multiplier, linestyle="--", color="green")
    bv.axhline(pov_low * pov_multiplier, linestyle="--", color="red")
    bv.plot(x, [row[1][6] for row in data], label=u"5秒間隔", linewidth=0.5)
    bv.plot(x, [row[1][7] for row in data], label=u"5分平均", linewidth=2,color="r")

    soc = bv.twinx()
    soc.set_ylabel("SoC(%)")
    soc.set_ylim(0, 100)
    soc.xaxis.set_major_locator(xloc)
    soc.xaxis.set_major_formatter(xfmt)
    soc.fill_between(x, [row[1][16] if row[1][16] is not None else 0 for row in data], linewidth=1,color="g",zorder=2,alpha=0.1,label="SoC")

    bv.legend(loc="best")

    kwh.set_ylabel(u"当日の電力量(kWh)")
    #kwh.set_ylim(0.0, 1.0)
    #kwh.tick_params(labelsize=8)
    kwh.grid(True)
    kwh.plot(x, [row[1][14] for row in data], label=u"発電", color="g")
    kwh.plot(x, [row[1][15] for row in data], label=u"消費", color="r")
    kwh.plot(x, [row[1][14] - row[1][15] for row in data], label=u"充電(発電-消費)", color="b")
    kwh.axhline(0.0, linestyle="-", color="grey")
    kwh.legend(loc="best")

    temp.set_ylabel(u"温度(℃)")
    temp.grid(True)
    temp.plot(x, [row[1][12] for row in data], label=u"5秒間隔", linewidth=0.5)
    temp.plot(x, [row[1][13] for row in data], label=u"5分平均", linewidth=2,color="r")
    temp.legend(loc="best")

    buf = io.BytesIO()
    fig.savefig(buf, format="png")
    buf.seek(0)
    return buf.read()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--date", type = str, dest = "date", default = None)
    parser.add_argument("-o", "--output", type = str, dest = "output", default="graph.png")
    parser.add_argument("-n", "--nodename", type = str, dest = "nodename", default=socket.gethostname())
    args = parser.parse_args()

    pov_ymin, pov_ymax, pov_middle, pov_low = (None, None, None, None)
    with database.Connection() as cur:
        cur.execute("select battery_type from nodes where nodename=%s", (args.nodename,))
        row = cur.fetchone()
        if row is not None and row[0] == 4: # 3S/6S
            pov_ymin, pov_ymax, pov_middle, pov_low = (9.0, 13.0, 12.0, 9.44)

    graph = generate_graph(args.nodename, args.date, pov_ymin, pov_ymax, pov_middle, pov_low)
    if graph is not None:
        with open(args.output, "w") as f:
            f.write(graph)
