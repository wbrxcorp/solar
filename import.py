#!/usr/bin/python
import sys,MySQLdb

'''
create table data(
	t datetime primary key,
	piv float,
	pia float,
	piw float,
	pov float,
	poa float,
	loadw float,
	temp float,
	kwh float,
	lkwh float
);

alter table data add column hostname varchar(32) not null default 'motion' FIRST;
alter table data drop primary key,add primary key(hostname,t), change column hostname hostname varchar(32) not null;
create index t_idx using btree on data(t);
alter table data add column soc float;
alter table data add column aiv float,add column aia float,add column aiw float;

create table nodes(
	nodename varchar(64) primary key,
	battery_type int,
	battery_capacity int
);
alter table nodes add column gpio bool;

create table schedule(
	id serial primary key,
	nodename varchar(64) not null,
	t datetime,
	`key` varchar(16) not null,
	int_value int,
	created_at datetime not null,
	unique(nodename,t,`key`)
);

create table bv_conditions(
	nodename varchar(64) not null,
	`key` varchar(16) not null,
	int_value int,
	lt float,
	gt float,
	primary key(nodename,`key`,int_value)
);

create table conditions(
	nodename varchar(64) not null,
	`key` varchar(16) not null,
	expression text,
	primary key(nodename,`key`)
);
-- replace into conditions(nodename,`key`,expression) values('kennel04','pw1','1 if pw1==0 and piw>100 and pov>14.2 else 0 if pw1==1 and pov<12.0 else None');

'''

hostname = 'motion'

conn = MySQLdb.connect(db="solar")
conn.autocommit(False)

cur = conn.cursor()

for line in iter(sys.stdin.readline, ""):
	if line[0] == '#': continue
	splitted = line.strip().split('\t')
	cur.execute("replace into data(hostname,t,piv,pia,piw,pov,poa,loadw,temp,kwh,lkwh) values(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)", (hostname,splitted[0], float(splitted[1]),float(splitted[2]),float(splitted[3]),float(splitted[4]),float(splitted[5]),float(splitted[6]),float(splitted[7]),float(splitted[8]),float(splitted[9])))

conn.commit()
cur.close()
conn.close()
