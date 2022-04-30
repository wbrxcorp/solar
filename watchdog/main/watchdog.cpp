#include <iostream>
#include <cstring>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "ping/ping_sock.h"
#include "driver/uart.h"

#include "config.h"
#include "console.h"

static const char* TAG = "watchdog";
static esp_ping_handle_t ping = NULL;

static void watchdog(void* pvParameters)
{
    auto semaphore = (SemaphoreHandle_t)pvParameters;
    while(xSemaphoreTake(semaphore, (1000LL * config::timeout) / portTICK_PERIOD_MS)) {
        ;
    }

    // semaphore timeout
    ESP_LOGW(TAG, "Vow Wow!!");
    gpio_set_level(config::gpio_pin, 1); // relay on (power off)
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    gpio_set_level(config::gpio_pin, 0); // relay off (power on)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_sleep_enable_timer_wakeup(config::sleep_duration * 1000LL * 1000LL);
    esp_deep_sleep_start();
}

static void on_ping_success(esp_ping_handle_t hdl, void *args)
{
    auto semaphore = (SemaphoreHandle_t)args;
    ESP_LOGI(TAG, "Ping success");
    esp_sleep_enable_timer_wakeup(config::sleep_duration * 1000LL * 1000LL);
    esp_deep_sleep_start();
    xSemaphoreGive(semaphore);
}

extern "C" {
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, 115200));

    // Initialize GPIO
    {
        gpio_config_t config;
        config.intr_type = GPIO_INTR_DISABLE;
        config.mode = GPIO_MODE_OUTPUT;
        config.pin_bit_mask = (1ULL << config::gpio_pin);
        config.pull_down_en = GPIO_PULLDOWN_DISABLE;
        config.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&config));
    }

    bool cmdline_only_mode = false;
    config::load();
    if (config::wifi.sta.ssid[0] == '\0') cmdline_only_mode = true;
    else if (esp_reset_reason() != ESP_RST_DEEPSLEEP){
        puts("Send ESC to enter command line only mode.");
        for (int i = 3; i > 0; i--) {
            printf("%d...", i);
            fflush(stdout);
            char c;
            while (uart_read_bytes(UART_NUM_0, &c, 1, 1000 / portTICK_PERIOD_MS) > 0) {
                if (c == 0x1b) {
                    cmdline_only_mode = true;
                    goto out;
                }
            }
        }
        out:;
        puts("\n");
    }

    SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();

    if (!cmdline_only_mode) {
        // Establish network connection
        ESP_ERROR_CHECK(esp_netif_init());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
            [](void* arg, esp_event_base_t, int32_t event_id, void*) {
                if (event_id == WIFI_EVENT_STA_START) {
                    esp_wifi_connect();
                } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
                    vTaskDelay(10000 / portTICK_PERIOD_MS);
                    esp_wifi_connect();
                } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
                    esp_netif_create_ip6_linklocal((esp_netif_t*)arg);
                }
            }, esp_netif_create_default_wifi_sta()));

        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
            [](void* arg, esp_event_base_t, int32_t, void*) {
                if (ping != NULL) return;
                //else
                esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
                ipaddr_aton(config::ping_host, &ping_config.target_addr);
                ping_config.count = ESP_PING_COUNT_INFINITE;
                ping_config.interval_ms = 5000;
                esp_ping_callbacks_t cbs;
                memset(&cbs, 0, sizeof(cbs));
                cbs.on_ping_success = on_ping_success;
                cbs.cb_args = arg;
                ESP_ERROR_CHECK(esp_ping_new_session(&ping_config, &cbs, &ping));
                ESP_ERROR_CHECK(esp_ping_start(ping));
            }, semaphore));

        config::wifi.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config::wifi) );
        ESP_ERROR_CHECK(esp_wifi_start() );

        TaskHandle_t watchdog_task;
        xTaskCreatePinnedToCore(watchdog, "watchdog", 2048, semaphore, 1, &watchdog_task, 0);
   }
 
    console::setup();
    console::watchdog_semaphore = semaphore;
    while (true) {
        if (!console::process(">")) break;
    }
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    puts("System halted");
}
}