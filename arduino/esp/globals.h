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
#define OPERATION_MODE_MAX 5

typedef struct strCONFIG {
  uint8_t default_operation_mode;
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} CONFIG;

#include "display.h"
extern Display display;

extern CONFIG config;

#include "edogawa_unit.h"
extern EdogawaUnit edogawaUnit1, edogawaUnit2;

#include "RS485Modbus.h"
extern RS485Modbus modbus;

extern bool debug_mode;

#endif // __GLOBALS_H__
