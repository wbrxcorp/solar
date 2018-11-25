#include "globals.h"

uint8_t operation_mode = OPERATION_MODE_NORMAL;
CONFIG config;

EdogawaUnit edogawaUnit1, edogawaUnit2;

RS485Modbus modbus;

Display display;

bool debug_mode = false;
