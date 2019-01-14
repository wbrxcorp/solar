#ifndef __RS485_MODBUS_H__
#define __RS485_MODBUS_H__

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
#include <driver/uart.h>
//#include <soc/uart_struct.h>
#endif

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
#if defined(ARDUINO_ARCH_ESP8266)
  int commPin, rtsPin;
#elif defined(ARDUINO_ARCH_ESP32)
  uart_port_t uartNum;
#endif
  int rtrPin;

  uint32_t speed;
//  uint32_t m_bitTime;
  uint16_t modbusTimeoutInMs;

//  int modbusTimeoutInMs;
//  uint32_t modbusTimeout;
  bool slave;
public:
  RS485Modbus() :
#if defined(ARDUINO_ARCH_ESP8266)
    commPin(-1), rtsPin(-1),
#elif defined(ARDUINO_ARCH_ESP32)
    uartNum(UART_NUM_1),
#endif
    rtrPin(-1), slave(false) {;}
#if defined(ARDUINO_ARCH_ESP8266)
  void begin(int _commPin, int _rtsPin, int _rtrPin/*-1 not to use*/, uint32_t speed, int _modbusTimeout);
#elif defined(ARDUINO_ARCH_ESP32)
  void begin(uart_port_t _uartNum, int txPin, int rxPin, int rtsPin, int _rtrPin/*-1 not to use*/, uint32_t speed, int _modbusTimeout);
#endif
  void enter_slave();
  void send_modbus_message(const uint8_t* message, size_t size);
  int ICACHE_RAM_ATTR receive_modbus_message(uint8_t* modbus_message/*must have 255 bytes at least*/);
};

#endif  // __RS485_MODBUS_H__
