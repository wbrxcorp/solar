#include "mqtt.h"
#include <cstring>
#include <string>
#include <map>
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
        sprintf(buf, "\td:%d", values.d); message += buf;
        sprintf(buf, "\tt:%d", values.t); message += buf;
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

static void process_message(const std::string& message)
{
    if (message.length() < 3 || strncmp(message.c_str(), "OK\t", 3) != 0) return;

    std::map<std::string,std::string> values;
    const char* pt = message.c_str() + 3;
    while (*pt) {
        const char* ptcolon = strchr(pt, ':');
        if (ptcolon == NULL) break; // end parsing string if there's no colon anymore
        char key[ptcolon - pt + 1];
        strncpy(key, pt, ptcolon - pt);
        key[ptcolon - pt] = '\0';
        pt = ptcolon + 1;
        const char* ptdelim = strchr(pt, '\t');
        if (ptdelim == NULL) ptdelim = strchr(pt, '\0');
        // ptdelim can't be NULL here
        char value[ptdelim - pt + 1];
        strncpy(value, pt, ptdelim - pt);
        value[ptdelim - pt] = '\0';
        pt = (*ptdelim != '\0') ? ptdelim + 1 : ptdelim;
        values[key] = value;
    }

    int32_t date = -1, time = -1;
    uint16_t sleep_sec = 0;

    for (auto i = values.begin(); i != values.end(); i++) {
        const auto& key = i->first;
        const auto& value = i->second;
        if (key == "d") {
            date = std::stol(value);
        } else if (key == "t") {
            time = std::stol(value);
        } else if (key == "bt") {
            epsolar::set_battery_type(std::stoi(value));
        } else if (key == "bc") {
            epsolar::set_battery_capacity(std::stoi(value));
        } else if (key == "pw") {
            epsolar::set_load_switch(std::stoi(value) > 0);
        } else if (key == "sleep") {
            sleep_sec = (int16_t)std::stoi(value);
        } else {
            ESP_LOGI(TAG, "%s=%s\n", i->first.c_str(), i->second.c_str());
        }
    }

    if (date > 20220101L && time >= 0) epsolar::set_datetime(date, time);

    if (sleep_sec > 0) {
        stop_sending_data();
        deepsleep::sleep_and_restart(sleep_sec);
    }
}

static std::string my_topic_name()
{
    return std::string("solar") + '/' + config::nodename;
}

static void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto event = (esp_mqtt_event_handle_t)event_data;
    if (event->event_id == MQTT_EVENT_ERROR) {
        ESP_LOGI(TAG, "MQTT Error");
    } else if (event->event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Connected.");
        mqtt_retry_count = 0;
        deepsleep::reset_sleep_duration();
        auto topic = my_topic_name();
        esp_mqtt_client_subscribe(mqtt_client, topic.c_str(), 0);
    } else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
        stop_sending_data();
        mqtt_retry_count++;
        ESP_LOGI(TAG, "MQTT Disonnected count=%d.", mqtt_retry_count);
        if (mqtt_retry_count >= config::MAX_MQTT_RETRY_COUNT) deepsleep::sleep_and_restart();
    } else if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
        if (esp_reset_reason() != ESP_RST_DEEPSLEEP) {
            std::string message("JOIN\tnodename:");
            message += config::nodename;
            esp_mqtt_client_publish(mqtt_client, config::MQTT_HUB_TOPIC, message.c_str(), 0, 0, 0);
        }
        start_sending_data();
    } else if (event->event_id == MQTT_EVENT_DATA) {
        auto topic = std::string(event->topic, event->topic_len);
        if (topic == my_topic_name()) {
            auto data = std::string(event->data, event->data_len);
            process_message(data);
        }
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

void stop_client()
{
    stop_sending_data();
    if (mqtt_client) {
        esp_mqtt_client_disconnect(mqtt_client);
        esp_mqtt_client_stop(mqtt_client);
    }
}

} // namespace mqtt