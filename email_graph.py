#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse,smtplib,datetime,socket
import email.mime.multipart,email.mime.text,email.mime.image,email.header

import graph

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("gmail_addr", type = str)
    parser.add_argument("gmail_password", type = str)
    parser.add_argument("recipient_addr", type = str)
    parser.add_argument("-n", "--nodename", type = str, default = socket.gethostname())
    parser.add_argument("-d", "--date", type = str, dest = "date", default = None)
    args = parser.parse_args()
    nodename = args.nodename

    date = args.date if args.date is not None else (datetime.date.today() - datetime.timedelta(1)).strftime("%Y-%m-%d")

    msg = email.mime.multipart.MIMEMultipart()
    msg["From"] = args.gmail_addr
    msg["To"] = args.recipient_addr
    msg["Subject"] = email.header.Header(u"%s %s のグラフ" % (nodename,date), "utf-8")
    msg.attach(email.mime.text.MIMEText("グラフ", "plain", "utf-8"))
    imgpart = email.mime.image.MIMEImage(graph.generate_graph(nodename,date), "png")
    imgpart["Content-Disposition"] = 'attachment; filename="graph%s.png"' % date
    msg.attach(imgpart)

    server = smtplib.SMTP('smtp.gmail.com:587')
    server.ehlo()
    server.starttls()

    server.login(args.gmail_addr,args.gmail_password)
    server.sendmail(args.gmail_addr, [args.recipient_addr], msg.as_string())
    server.quit()
