#include <cstring>
#include "console.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"

#include "watchdog.h"
#include "config.h"

static const char* TAG = "console";

static int ssid(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [ssid]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        puts((const char*)config::wifi.sta.ssid);
        return 0;
    }
    //else
    if (strlen(argv[1]) >= sizeof(config::wifi.sta.ssid)) {
        puts("ssid too long");
        return 1;
    }
    //else
    strcpy((char*)config::wifi.sta.ssid, argv[1]);
    return 0;
}

static int password(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [password]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        puts((const char*)config::wifi.sta.password);
        return 0;
    }
    //else
    if (strlen(argv[1]) >= sizeof(config::wifi.sta.password)) {
        puts("passoword too long");
        return 1;
    }
    //else
    strcpy((char*)config::wifi.sta.password, argv[1]);
    return 0;
}

static int ping_host(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [ping_host]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        puts(config::ping_host);
        return 0;
    }
    //else
    if (strlen(argv[1]) >= sizeof(config::ping_host)) {
        puts("ping_host too long. it must be ipv4 address");
        return 1;
    }
    //else
    strcpy(config::ping_host, argv[1]);
    return 0;
}

static int use_sleep(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [0|1]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        puts(config::use_sleep? "1" : "0");
        return 0;
    }
    //else
    
    config::use_sleep = std::atoi(argv[1]) != 0? true : false;
    return 0;
}

namespace console {

SemaphoreHandle_t watchdog_semaphore = NULL;

void setup()
{
    setvbuf(stdin, NULL, _IONBF, 0);
    esp_vfs_dev_uart_port_set_rx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_port_set_tx_line_endings(UART_NUM_0, ESP_LINE_ENDINGS_CRLF);
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    esp_console_config_t console_config;
    console_config.max_cmdline_args = 8;
    console_config.max_cmdline_length = 256;
    ESP_ERROR_CHECK( esp_console_init(&console_config) );
    linenoiseSetMultiLine(1);
    linenoiseHistorySetMaxLen(100);

    esp_console_cmd_t cmd;
    cmd.command = "reset";
    cmd.help = "reset system";
    cmd.hint = NULL;
    cmd.func = [](int,char**) { esp_restart(); return 0;};
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "ssid";
    cmd.help = "get/set WiFi ssid";
    cmd.hint = NULL;
    cmd.func = ssid;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "password";
    cmd.help = "get/set WiFi password";
    cmd.hint = NULL;
    cmd.func = password;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "ping_host";
    cmd.help = "get/set ping host";
    cmd.hint = NULL;
    cmd.func = ping_host;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "save";
    cmd.help = "save config";
    cmd.hint = NULL;
    cmd.func = [](int,char**) { return config::save()? 0 : 1; };
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "use_sleep";
    cmd.help = "get/set sleep flag";
    cmd.hint = NULL;
    cmd.func = use_sleep;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "feed";
    cmd.help = "feed watchdog";
    cmd.hint = NULL;
    cmd.func = [](int,char**) {
        if (!semaphore) {
            ESP_LOGE(TAG, "Watchdog semaphore is not set");
            return pdFALSE;
        }
        return xSemaphoreGive(semaphore)? 0 : 1;
    };
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "disconnect";
    cmd.help = "disconnect wifi";
    cmd.hint = NULL;
    cmd.func = [](int,char**) {
        return esp_wifi_disconnect() == ESP_OK? 0 : 1;
    };
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

bool process(const char* prompt)
{
    char* line = linenoise(prompt);
    if (line == NULL || line[0] == '\0') return true;
    //else
    puts("");

    if (strcmp(line, "halt") == 0) {
        return false; // halt system
    }

    int ret;
    auto err = esp_console_run(line, &ret);

    if (err == ESP_ERR_INVALID_ARG) {
        puts("Invalid argument");
    } else if (err == ESP_ERR_NOT_FOUND) {
        puts("Command not found");
    } else {
        ESP_ERROR_CHECK(err);
    }

    linenoiseHistoryAdd(line);
    linenoiseFree(line);
    return true;
}

}// namespace console