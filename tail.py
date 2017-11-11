#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time,datetime,argparse
import database

def print_latest_data(nodename, since):
    row = None
    with database.Connection() as cur:
        cur.execute("select t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh from data where hostname=%s and t>%s order by t", (nodename,since))

    last_t = since
    num_rows = 0
    for row in cur:
        t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh = row
        last_t = t
        num_rows += 1
        print "%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f" % (t.strftime("%Y-%m-%d %H:%M:%S"),piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh)
    return last_t,num_rows

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("nodename", type = str)
    args = parser.parse_args()

    cnt = 0
    print_hdr = True
    since = datetime.datetime.now() - datetime.timedelta(seconds=60)
    while True:
        if print_hdr:
            print "# nodename: %s" % args.nodename
            print "# t\t\t\tpiv\tpia\tpiw\tpov\tpoa\tloadw\ttemp\tkwh\tlkwh"
            print_hdr = False
        since,num_rows = print_latest_data(args.nodename, since)
        time.sleep(1)
        cnt += num_rows
        if cnt >= 20:
            cnt = 0
            print_hdr = True
