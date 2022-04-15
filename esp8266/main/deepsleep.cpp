#include "deepsleep.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_attr.h"

#include "config.h"

static const char* TAG = "deepsleep";
RTC_DATA_ATTR static uint16_t sleep_duration = 1;

namespace deepsleep {

void sleep_and_restart()
{
    sleep_duration *= 2;
    ESP_LOGI(TAG, "Retry count exceeded. sleep for %d seconds.", (int)sleep_duration);
    if (sleep_duration > config::MAX_SLEEP_DURATION) sleep_duration = config::MAX_SLEEP_DURATION;
    esp_deep_sleep(sleep_duration * 1000L/*milli*/ * 1000L/*micro*/);
}

void sleep_and_restart(int second)
{
    esp_deep_sleep(second * 1000L/*milli*/ * 1000L/*micro*/);
}

void reset_sleep_duration()
{
    sleep_duration = 1;
}

}