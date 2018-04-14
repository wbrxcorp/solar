#ifndef __NETWORK_H__
#define __NETWORK_H__

void set_message_timeout(unsigned long timeout);
void connect(const char* additional_init_params = NULL);
void disconnect();
bool connected();
bool send_message(const char* message, void (*process_message)(const char*));

#endif // __NETWORK_H__
