#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "esp_wifi.h"

namespace config {
    extern char nodename[16];
    extern wifi_config_t wifi;
    extern const char* country;

    const uint16_t MAX_SLEEP_DURATION = 5 * 60;
    const uint8_t MAX_WIFI_RETRY_COUNT = 5;
    const uint8_t MAX_MQTT_RETRY_COUNT = 3;
    extern const char* MQTT_HUB_TOPIC;
    const uint16_t DATA_DURATION = 5;

    bool load();
    bool save();
}

#endif //__CONFIG_H__