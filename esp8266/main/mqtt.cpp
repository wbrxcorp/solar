#include "mqtt.h"
#include <cstring>
#include <string>
#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"

#include "config.h"
#include "deepsleep.h"
#include "epsolar.h"

static const char* TAG = "mqtt";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static uint8_t mqtt_retry_count = 0;
static esp_timer_handle_t timer = NULL;

static void timer_callback(void *arg)
{
    if (!mqtt_client) return;

    wifi_ap_record_t ap_info;
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));

    epsolar::Values values;
    if (epsolar::read_values(values)) {
        auto message = (std::string)"DATA\tnodename:" + config::nodename;
        char buf[32];
        sprintf(buf, "\tpiv:%.2f", values.piv); message += buf;
        sprintf(buf, "\tpia:%.2f", values.pia); message += buf;
        sprintf(buf, "\tpiw:%.2f", values.piw); message += buf;
        sprintf(buf, "\tbv:%.2f", values.bv); message += buf;
        sprintf(buf, "\tpoa:%.2f", values.poa); message += buf;
        sprintf(buf, "\tload:%.2f", values.load); message += buf;
        sprintf(buf, "\ttemp:%.2f", values.temp); message += buf;
        sprintf(buf, "\tlkwh:%.2f", values.lkwh); message += buf;
        sprintf(buf, "\tkwh:%.2f", values.kwh); message += buf;
        sprintf(buf, "\tpw:%d", values.pw? 1 : 0); message += buf;
        //sprintf(buf, "\tbtcv:%.2f", values.btcv); message += buf;
        //sprintf(buf, "\tpw1:%d", values.pw1? 1 : 0); message += buf;
        //sprintf(buf, "\tpw2:%d", values.pw2? 1 : 0); message += buf;
        sprintf(buf, "\trssi:%d", (int)ap_info.rssi); message += buf;
        sprintf(buf, "\tcs:%d", values.cs); message += buf;
        sprintf(buf, "\tsoc:%d", (int)values.soc); message += buf;
        //sprintf(buf, "\tgpio:%d", 0); message += buf;
        esp_mqtt_client_publish(mqtt_client, config::MQTT_HUB_TOPIC, message.c_str(), 0, 0, 0);
    } else {
        auto message = (std::string)"NODATA\tnodename:" + config::nodename;
        esp_mqtt_client_publish(mqtt_client, config::MQTT_HUB_TOPIC, message.c_str(), 0, 0, 0);
    }
}

static void start_sending_data()
{
    if (timer) return;
    //else
    timer_callback(NULL);

    esp_timer_create_args_t timer_args;
    timer_args.callback = timer_callback;
    timer_args.arg = NULL;
    timer_args.dispatch_method = ESP_TIMER_TASK;
    timer_args.name = "timer";
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, config::DATA_DURATION * 1000 * 1000));
}

static void stop_sending_data()
{
    if (!timer) return;
    ESP_ERROR_CHECK(esp_timer_delete(timer));
    timer = NULL;
}

static void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto event = (esp_mqtt_event_handle_t)event_data;
    if (event->event_id == MQTT_EVENT_ERROR) {
        ESP_LOGI(TAG, "MQTT Error");
    } else if (event->event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Connected.");
        mqtt_retry_count = 0;
        deepsleep::reset_sleep_duration();
        char topic[sizeof(config::nodename) + 6];
        strcpy(topic, "solar/");
        strcat(topic, config::nodename);
        esp_mqtt_client_subscribe(mqtt_client, topic, 0);
    } else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
        stop_sending_data();
        mqtt_retry_count++;
        ESP_LOGI(TAG, "MQTT Disonnected count=%d.", mqtt_retry_count);
        if (mqtt_retry_count >= config::MAX_MQTT_RETRY_COUNT) deepsleep::sleep_and_restart();
    } else if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
        char message[sizeof(config::nodename) + 5];
        strcpy(message, "JOIN\t");
        strcat(message, config::nodename);
        esp_mqtt_client_publish(mqtt_client, config::MQTT_HUB_TOPIC, message, 0, 0, 0);
        start_sending_data();
    } else if (event->event_id == MQTT_EVENT_DATA) {
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
    } else if (event->event_id == MQTT_EVENT_BEFORE_CONNECT) {
        // attemting connect
    } else {
        ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    }
}

namespace mqtt {

void start_client(const char* host, int port/* = 1883*/)
{
    if (mqtt_client) return;
    // else
    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
    mqtt_cfg.host = host;
    mqtt_cfg.port = port;
    mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, 
        ::event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}

} // namespace mqtt