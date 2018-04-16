#include "globals.h"

uint8_t operation_mode = OPERATION_MODE_NORMAL;
CONFIG config;

EdogawaUnit edogawaUnit;

EPSolar epsolar;

Adafruit_SSD1306 display(-1);

bool debug_mode = false;
