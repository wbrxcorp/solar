#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,argparse
import database

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("nodename", type = str)
    parser.add_argument("pw", type = int, help="0:off 1:on")
    args = parser.parse_args()

    if args.pw != 0 and args.pw != 1:
        print "pw must be 0(off) or 1(on)!"
        sys.exit()

    with database.Connection() as cur:
        cur.execute("select count(*) from nodes where nodename=%s", (args.nodename,))
        if cur.fetchone()[0] == 0:
            print "No such node defined"
            sys.exit()
        cur.execute("replace into schedule(nodename,`key`,int_value,created_at) values(%s,'pw',%s,now())", (args.nodename, args.pw))

    print "Done."
