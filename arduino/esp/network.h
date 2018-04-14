#ifndef __NETWORK_H__
#define __NETWORK_H__

bool connect(const char* nodename, const char* servername, uint16_t port);
void disconnect();
bool connected();
bool send_message(const char* message);
int receive_message(void (*process_message)(const char*));

#endif // __NETWORK_H__
