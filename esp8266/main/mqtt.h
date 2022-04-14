#ifndef __MQTT_H__
#define __MQTT_H__

#include "esp_event.h"

namespace mqtt {

void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void start_client(const char* host, int port = 1883);

} // namespace mqtt

#endif