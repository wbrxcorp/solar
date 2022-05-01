#include <cstring>
#include "config.h"

#include "nvs.h"

namespace config {

const char* NVS_PARTITION_NAME = "solar";
wifi_config_t wifi;
uint32_t timeout = 60 * 10; // 10 min
gpio_num_t gpio_pin = GPIO_NUM_13;
char ping_host[16] = "8.8.8.8";
uint32_t ping_interval = 10000;
bool use_sleep = true;
uint32_t sleep_duration = 60 * 10; // 10 min

bool load()
{
    nvs_handle_t nvs;
    auto err = nvs_open(NVS_PARTITION_NAME, NVS_READONLY, &nvs);

    if (err == ESP_OK) {
        auto ssid_size = sizeof(wifi.sta.ssid);
        err = nvs_get_str(nvs, "ssid", (char*)wifi.sta.ssid, &ssid_size);
        if (err == ESP_OK) {
            auto password_size = sizeof(wifi.sta.password);
            err = nvs_get_str(nvs, "password", (char*)wifi.sta.password, &password_size);
        }

        auto ping_host_size = sizeof(ping_host);
        nvs_get_str(nvs, "ping_host", ping_host, &ping_host_size);

        uint8_t b;
        if (nvs_get_u8(nvs, "use_sleep", &b) == ESP_OK) config::use_sleep = (b != 0);

        nvs_close(nvs);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND) return false;
    ESP_ERROR_CHECK(err);

    return true;
}

bool save()
{
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open(NVS_PARTITION_NAME, NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ssid", (const char*)wifi.sta.ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "password", (const char*)wifi.sta.password));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ping_host", ping_host));
    ESP_ERROR_CHECK(nvs_set_u8(nvs, "use_sleep", config::use_sleep? 1 : 0));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
    return true;
}

} // namespace config

