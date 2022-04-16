#include <string>
#include <cstring>
#include "console.h"
#include "config.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"

#include "mqtt.h"
#include "wifi.h"

static int nodename(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [nodename]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        puts(config::nodename);
        return 0;
    }
    //else
    if (strlen(argv[1]) >= sizeof(config::nodename)) {
        puts("nodename too long");
        return 1;
    }
    //else
    strcpy(config::nodename, argv[1]);
    return 0;
}

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

static int country(int argc, char** argv)
{
    if (argc > 2) {
        puts("Usage:");
        printf("  %s [country or none]", argv[0]);
        return 1;
    }
    //else
    if (argc == 1) {
        printf("%s\n", config::country? config::country : "none");
        return 0;
    }
    //else
    if (strcmp(argv[1], "none") == 0) {
        config::country = NULL;
        return 0;
    }
    //else
    if (strcmp(argv[1], "JP") == 0) {
        config::country = "JP";
        return 0;
    }
    //else
    puts("Unknown country code");
    return 1;
}

static int save(int argc, char** argv)
{
    return config::save()? 0 : 1;
}

namespace console {

void setup()
{
    setvbuf(stdin, NULL, _IONBF, 0);
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
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

    cmd.command = "nodename";
    cmd.help = "get/set nodename";
    cmd.hint = NULL;
    cmd.func = nodename;
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

    cmd.command = "country";
    cmd.help = "get/set WiFi country code";
    cmd.hint = NULL;
    cmd.func = country;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));

    cmd.command = "save";
    cmd.help = "save config";
    cmd.hint = NULL;
    cmd.func = save;
    cmd.argtable = NULL;
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

bool process(const char* prompt)
{
    char* line = linenoise(prompt);
    if (line == NULL) return true;
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

}