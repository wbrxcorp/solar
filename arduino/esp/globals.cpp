#include "globals.h"

uint8_t operation_mode = OPERATION_MODE_NORMAL;
CONFIG config;

EdogawaUnit edogawaUnit1, edogawaUnit2;

RS485Modbus modbus;

Display display;

TFT tft;

bool debug_mode = false;

Adafruit_BME280 bme; // I2C
Adafruit_CCS811 ccs;
