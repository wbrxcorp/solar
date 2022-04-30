#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace console {

extern SemaphoreHandle_t watchdog_semaphore;

void setup();
bool process(const char* prompt);

}

#endif // __CONSOLE_H__