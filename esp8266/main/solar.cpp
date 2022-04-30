// solar.cpp
#include <string>
#include <cstring>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "mdns.h"

#include "config.h"
#include "console.h"
#include "modbus_esp8266.h"
#include "epsolar.h"
#include "wifi.h"
#include "mqtt.h"
#include "deepsleep.h"
#include "service_discovery.h"

static const char* TAG = "solar";

static bool wait_for_escape_key()
{
    puts("Send ESC to enter command line only mode.");
    for (int i = 3; i > 0; i--) {
        printf("%d...", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_RATE_MS);
        size_t size;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, &size));
        char buf[size];
        auto read_size = uart_read_bytes(UART_NUM_0, (uint8_t*)buf, sizeof(buf), 0);
        for (int i = 0; i < read_size; i++) {
            if (buf[i] == 0x1b) {
                puts("");
                return true;
            }
        }
    }
    puts("\n");
    return false;
}

void real_main()
{
    printf("SDK version : %s\n", esp_get_idf_version());
    printf("Build date  : %s %s\n", __DATE__, __TIME__);

    ESP_ERROR_CHECK(nvs_flash_init());

    modbus_esp8266::Driver modbus_driver;
    modbus_driver.init();
    modbus::init(modbus_driver);

    bool cmdline_only_mode = false;
    if (config::load()) {
        printf("nodename    : %s\n", config::nodename);
        printf("ssid        : %s\n", config::wifi.sta.ssid);
    } else {
        puts("\nnodename, ssid, password not initialized. Entering command line only mode.");
        cmdline_only_mode = true;
    }

    auto reset_reason = esp_reset_reason();

    if (reset_reason != ESP_RST_DEEPSLEEP) {
        modbus::DeviceInfo modbus_device_info;
        if (modbus::get_basic_device_info(modbus_device_info)) {
            printf("Vendor      : %s\n", modbus_device_info.get_vendor_name());
            printf("Product     : %s\n", modbus_device_info.get_product_code());
            printf("Revision    : %s\n", modbus_device_info.get_revision());
        }
        epsolar::Settings settings;
        if (epsolar::read_settings(settings)) {
            printf("Battery type: %d(%s), %dAh\n", settings.battery_type, settings.battery_type_str(),
            settings.battery_capacity);
            if (settings.battery_type == 0/*User defined*/) {
                printf("  Boost Charging Voltage: %.2fV", settings.boost_voltage/* * multiplier*/);
                printf("  Float Charging Voltage: %.2fV", settings.float_voltage/* * multiplier*/);
                printf("  Low Voltage Disconnect: %.2fV", settings.low_voltage_disconnect/* * multiplier*/);
            }

            printf("Battery real rated voltage: %dV\n", (int)settings.battery_rated_voltage);
            printf("Temperature compensation coefficient: %dmV/Cecelsius degree/2V\n", (int)settings.temperature_compensation_coefficient);
        } else {
            ESP_LOGE(TAG, "Reading battery settings from charge controller failed.");
        }
        if (modbus::write_query(1, 6, 0x903d/*Load controlling mode*/, (uint16_t)0)) {
            puts("Load controlling mode set to 0(Manual)");
        } else ESP_LOGE(TAG, "Error setting load controlling mode(0x903d)");
        if (modbus::write_query(1, 6, 0x906a/*Default load on/off in manual mode*/, (uint16_t)1)) {
            puts("Default load on/off in manual mode set to 1(on)");
        } else ESP_LOGE(TAG, "Error setting default load on/off in manual modee(0x906a)");
    }

    if (!cmdline_only_mode && reset_reason != ESP_RST_DEEPSLEEP) {
        cmdline_only_mode = wait_for_escape_key();
    }

    if (!cmdline_only_mode) {
        tcpip_adapter_init();
        wifi::start();
        ESP_ERROR_CHECK( mdns_init() );
        ESP_ERROR_CHECK(mdns_hostname_set(config::nodename));
        auto services_found = service_discovery::discover("_mqtt", "_tcp", 1);
        if (services_found.size() > 0) {
            auto service = *services_found.begin();
            auto host = service.first.c_str();
            auto port = service.second;
            ESP_LOGI(TAG, "Service '_mqtt._tcp' discovered: addr=%s, port=%d\n", host, port);
            mqtt::start_client(host, port);
        } else {
            ESP_LOGE(TAG, "MQTT service couldn't be resolved");
            deepsleep::sleep_and_restart();
        }
    }

    console::setup();

    while(true) {
        auto prompt = std::string(config::nodename) + ">";
        if (!console::process(prompt.c_str())) break;
    }

    mqtt::stop_client();
    wifi::stop();
    puts("System halted");
}

extern "C" { void app_main() {
    esp_set_cpu_freq(ESP_CPU_FREQ_160M);
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, 115200));
    puts("");
    real_main();
} }
