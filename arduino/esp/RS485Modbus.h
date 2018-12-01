#ifndef __RS485_MODBUS_H__
#define __RS485_MODBUS_H__

#include <Arduino.h>

#define MAX_MODBUS_MESSAGE_LENGTH 255

inline uint16_t MKWORD(uint8_t hi, uint8_t lo)
{
  return ((uint16_t)hi) << 8 | lo;
}

inline uint32_t MKDWORD(uint16_t hi, uint16_t lo)
{
  return ((uint32_t)hi) << 16 | lo;
}

class RS485Modbus {
  int commPin;
  unsigned long m_bitTime;
  unsigned long modbusTimeout;
  int rtsPin, rtrPin;
  bool slave;
public:
  RS485Modbus() : commPin(-1), rtsPin(-1), rtrPin(-1), slave(false) {;}
  void begin(int _commPin, int _rtsPin, int _rtrPin/*-1 not to use*/, long speed, int _modbusTimeout);
  void enter_slave();
  void send_modbus_message(const uint8_t* message, size_t size);
  int ICACHE_RAM_ATTR receive_modbus_message(uint8_t* modbus_message/*must have 255 bytes at least*/);
};

#endif  // __RS485_MODBUS_H__
