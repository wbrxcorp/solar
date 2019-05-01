#!/usr/bin/python2

import threading,socket
from zeroconf import Zeroconf, ServiceBrowser

class ServiceResolver:
  def __init__(self, zeroconf):
    self.zeroconf = zeroconf
    self.discover_complete = threading.Event()

  def resolve(self, type, timeout=10):
    browser = ServiceBrowser(self.zeroconf, type, self)
    self.discover_complete.wait(timeout)
    if not self.discover_complete.is_set(): return None
    # else
    return [self.ip_address,self.port]

  def add_service(self, zeroconf, type, name):
    service_info = zeroconf.get_service_info(type,name)
    self.ip_address = socket.inet_ntoa(service_info.address)
    self.port = service_info.port
    self.discover_complete.set()

def resolve(type):
  zeroconf = Zeroconf()
  try:
    resolver = ServiceResolver(zeroconf)
    return resolver.resolve(type)
  finally:
    zeroconf.close()

if __name__ == '__main__':
  print resolve("_mqtt._tcp.local.")
