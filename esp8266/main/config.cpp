#include <cstring>
#include "config.h"

#include "nvs.h"

namespace config {

const char* NVS_PARTITION_NAME = "solar";
char nodename[16] = "unknown";
wifi_config_t wifi;
const char* country = NULL;
const char* MQTT_HUB_TOPIC = "solar/hub";

bool load()
{
    nvs_handle_t nvs;
    auto err = nvs_open(NVS_PARTITION_NAME, NVS_READONLY, &nvs);

    if (err == ESP_OK) {
        auto nodename_size = sizeof(nodename);
        auto err = nvs_get_str(nvs, "nodename", nodename, &nodename_size);
        if (err == ESP_OK) {
            auto ssid_size = sizeof(wifi.sta.ssid);
            err = nvs_get_str(nvs, "ssid", (char*)wifi.sta.ssid, &ssid_size);
            if (err == ESP_OK) {
                auto password_size = sizeof(wifi.sta.password);
                err = nvs_get_str(nvs, "password", (char*)wifi.sta.password, &password_size);
            }
        }

        char buf[3];
        size_t buf_size = sizeof(buf);
        if (nvs_get_str(nvs, "country", buf, &buf_size) == ESP_OK) {
            if (strcmp(buf, "JP") == 0) country = "JP";
        }
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
    ESP_ERROR_CHECK(nvs_set_str(nvs, "nodename", nodename));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ssid", (const char*)wifi.sta.ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "password", (const char*)wifi.sta.password));
    if (country) {
        ESP_ERROR_CHECK(nvs_set_str(nvs, "country", country));
    } else {
        nvs_erase_key(nvs, "country");
    }
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
    return true;
}

} // namespace config