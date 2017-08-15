#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
[ebuild  N     ] media-libs/qhull-2012.1-r4::gentoo  USE="-doc -static-libs" 700 KiB
[ebuild  N     ] media-fonts/stix-fonts-1.1.1::gentoo  USE="-X" 1,792 KiB
[ebuild  N     ] dev-python/matplotlib-1.4.3::gentoo  USE="-cairo -doc -examples -excel -fltk -gtk -gtk3 -latex -pyside -qt4 -qt5 {-test} -tk -wxwidgets" PYTHON_TARGETS="python2_7 python3_4 (-python3_5) (-python3_6)" 48,764 KiB
"""

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

        cur.execute("select t,piv,pia,piw,pov,poa,bv,temp,kwh from data where t >= %s and t < %s order by t", (starttime,  endtime))
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
    piw.plot(x, [row[1][2] for row in data], linewidth=0.5)

    bv.set_ylabel(u"バッテリー電圧(V)")
    #bv.set_ylim(10, 15)
    #bv.tick_params(labelsize=8)
    bv.grid(True)
    bv.plot(x, [row[1][5] for row in data], linewidth=0.5)

    kwh.set_ylabel(u"当日発電した電力量(kWh)")
    #kwh.set_ylim(0.0, 1.0)
    #kwh.tick_params(labelsize=8)
    kwh.grid(True)
    kwh.plot(x, [row[1][7] for row in data])

    buf = io.BytesIO()
    fig.savefig(buf, format="png")
    buf.seek(0)
    return buf.read()

if __name__ == '__main__':
    graph = generate_graph(sys.argv[1] if len(sys.argv) > 1 else None)
    with open("graph.png", "w") as f:
        f.write(graph)
