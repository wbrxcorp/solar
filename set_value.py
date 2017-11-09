#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys,socket,argparse
import database

def set_value(nodename, key, value):
    with database.Connection() as cur:
        cur.execute("select count(*) from nodes where nodename=%s", (nodename,))
        if cur.fetchone()[0] == 0:
            print "No such node defined"
            return False
        cur.execute("replace into schedule(nodename,`key`,int_value,created_at) values(%s,%s,%s,now())", (nodename, key, value))
    return True

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("nodename", type = str)
    parser.add_argument("key", type = str)
    parser.add_argument("value", type = int)
    args = parser.parse_args()

    if set_value(args.nodename, args.key, args.value):
        print "Key %s set to %d." % (args.key, args.value)
