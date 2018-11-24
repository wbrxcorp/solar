#ifndef __RS485_MODBUS_H__
#define __RS485_MODBUS_H__

#include <Arduino.h>

#define MAX_MODBUS_MESSAGE_LENGTH 255

class RS485Modbus {
  int commPin;
  unsigned long m_bitTime;
  unsigned long modbusTimeout;
  int rtsPin, rtrPin;
public:
  RS485Modbus() : commPin(-1), rtsPin(-1), rtrPin(-1) {;}
  void begin(int _commPin, int _rtsPin, int _rtrPin/*-1 not to use*/, long speed, int _modbusTimeout);
  void send_modbus_message(const uint8_t* message, size_t size);
  int ICACHE_RAM_ATTR receive_modbus_message(uint8_t* modbus_message/*must have 255 bytes at least*/);
};

#endif  // __RS485_MODBUS_H__
