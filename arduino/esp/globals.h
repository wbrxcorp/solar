#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <Arduino.h>

extern uint8_t operation_mode;
#define OPERATION_MODE_COMMAND_LINE_ONLY 0
#define OPERATION_MODE_COMMAND_LINE 1
#define OPERATION_MODE_NORMAL 2
#define OPERATION_MODE_SERVER 3
#define OPERATION_MODE_NISETRACER 4
#define OPERATION_MODE_EDOGAWA_MASTER 5
#define OPERATION_MODE_THERMOMETER 6
#define OPERATION_MODE_AMMETER 7
#define OPERATION_MODE_SLAVE 8
#define OPERATION_MODE_WATCHDOG 9
#define OPERATION_MODE_MAX 9

typedef struct strCONFIG {
  uint8_t default_operation_mode;
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint8_t slave_id;
  uint8_t tft_rotation;
  uint16_t crc;
} CONFIG;

#include "display.h"
extern Display display;

#include "TFT.h"
extern TFT tft;

#include <Adafruit_BME280.h>
extern Adafruit_BME280 bme; // I2C

#include <Adafruit_CCS811.h>
extern Adafruit_CCS811 ccs;

#include <Adafruit_INA219.h>
extern Adafruit_INA219 ina219;

extern CONFIG config;

#include "edogawa_unit.h"
extern EdogawaUnit edogawaUnit1, edogawaUnit2;

#include "RS485Modbus.h"
extern RS485Modbus modbus;

extern bool debug_mode;

#endif // __GLOBALS_H__
