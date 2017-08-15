#!/usr/bin/python
import sys,MySQLdb

# create table data(t datetime primary key, piv float,pia float,piw float,pov float,poa float,bv float,temp float,kwh float)

conn = MySQLdb.connect(db="solar")
conn.autocommit(False)

cur = conn.cursor()

for line in iter(sys.stdin.readline, ""):
	if line[0] == '#': continue
	splitted = line.strip().split('\t')
	cur.execute("replace into data(t,piv,pia,piw,pov,poa,bv,temp,kwh) values(%s,%s,%s,%s,%s,%s,%s,%s,%s)", (splitted[0], float(splitted[1]),float(splitted[2]),float(splitted[3]),float(splitted[4]),float(splitted[5]),float(splitted[6]),float(splitted[7]),float(splitted[8])))

conn.commit()
cur.close()
conn.close()
