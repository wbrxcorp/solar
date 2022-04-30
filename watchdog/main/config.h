#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "esp_wifi.h"
#include "driver/gpio.h"
#include "lwip/dns.h"

namespace config {
    extern wifi_config_t wifi;
    extern uint32_t timeout;
    extern gpio_num_t gpio_pin;
    extern char ping_host[16];
    extern uint32_t sleep_duration;

    bool load();
    bool save();
}

#endif // __CONFIG_H__