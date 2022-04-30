#include <cstring>

#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "config.h"
#include "deepsleep.h"

static const char* TAG = "wifi";
static uint8_t wifi_retry_count = 0;
static EventGroupHandle_t wifi_event_group;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            wifi_retry_count = 0;
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_STOP) {
            // WiFi STA stopped
        } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
            wifi_retry_count = 0;
            tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_retry_count++;
            ESP_LOGI(TAG, "WiFi Disonnected count=%d.", wifi_retry_count);
            if (wifi_retry_count >= config::MAX_WIFI_RETRY_COUNT) deepsleep::sleep_and_restart();
            //else
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG, "Some WiFi event happened: %d", event_id);
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            // Got IPv4 address
        } else if (event_id == IP_EVENT_STA_LOST_IP) {
            // Lost IP address
        } else if (event_id == IP_EVENT_GOT_IP6) {
            //const ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
            ip6_addr_t& ipv6_addr = ((ip_event_got_ip6_t *)event_data)->ip6_info.ip;
            //memcpy(&ipv6_addr, &event->ip6_info.ip, sizeof(ipv6_addr));
            if (!ip6_addr_isglobal(&ipv6_addr)) { // link-local only
                auto ipv6str = ip6addr_ntoa(&ipv6_addr);
                ESP_LOGI(TAG, "Got IPv6 link-local address: %s , zone=%d\n", ipv6str, (int)ipv6_addr.zone);
                xEventGroupSetBits(wifi_event_group, 1);
            }
        } else {
            ESP_LOGI(TAG, "Some IP event happened: %d", event_id);
        }
    }
}

namespace wifi {

void start()
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    config::wifi.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config::wifi) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    if (config::country && strcmp(config::country, "JP") == 0) {
        // https://tech.smartshopping.co.jp/esp8266_rtos_3_4
        ESP_LOGI(TAG, "Country set to %s", config::country);
        wifi_country_t country_info;
        strcpy(country_info.cc, "JP");
        country_info.schan = 1;
        country_info.nchan = 13;
        country_info.policy = WIFI_COUNTRY_POLICY_MANUAL;
        ESP_ERROR_CHECK(esp_wifi_set_country(&country_info));
    }
    wifi_event_group = xEventGroupCreate();
    /*EventBits_t bits = */xEventGroupWaitBits(wifi_event_group, 1, pdFALSE, pdFALSE, portMAX_DELAY);
}

void stop()
{
    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) == ESP_OK) {
        esp_wifi_disconnect();
        esp_wifi_stop();
        esp_wifi_deinit();
    }
}

}