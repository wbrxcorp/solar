#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__


#include "esp_sleep.h"

namespace deepsleep {
    void sleep_and_restart();
    void reset_sleep_duration();
}

#endif // __DEEPSLEEP_H__