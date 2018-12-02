#ifndef __NETWORK_H__
#define __NETWORK_H__

bool connect(const char* nodename, const char* servername, uint16_t port);
void disconnect();
bool connected();
String get_remote_address();
uint16_t get_remote_port();
bool send_message(const char* message);
int receive_message(void (*process_message)(const char*));

#ifdef ARDUINO_ARCH_ESP8266
  bool start_mdns_if_not_yet();
#endif


#endif // __NETWORK_H__
