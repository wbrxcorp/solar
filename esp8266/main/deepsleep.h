#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__


#include "esp_sleep.h"

namespace deepsleep {
    void sleep_and_restart();
    void sleep_and_restart(int second);
    void reset_sleep_duration();
}

#endif // __DEEPSLEEP_H__