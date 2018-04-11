#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <Arduino.h>

extern uint8_t operation_mode;
#define OPERATION_MODE_NORMAL 0
#define OPERATION_MODE_COMMAND_LINE 1
#define OPERATION_MODE_COMMAND_LINE_ONLY 2

typedef struct strCONFIG {
  char nodename[32];
  char ssid[34];
  char key[64];
  char servername[48];
  uint16_t port;
  uint16_t crc;
} CONFIG;

extern CONFIG config;

#include "edogawa_unit.h"
extern EdogawaUnit edogawaUnit;

#include "epsolar.h"
extern EPSolar epsolar;

extern bool debug_mode;

#endif // __GLOBALS_H__
