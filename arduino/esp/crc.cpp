#include "crc.h"

uint16_t update_crc(uint16_t crc, uint8_t val)
{
  crc ^= (uint16_t)val;
  for (int i = 8; i != 0; i--) {
    if ((crc & 0x0001) != 0) {
      crc >>= 1;
      crc ^= 0xA001;
    } else {
      crc >>= 1;
    }
  }
  return crc;
}

void put_crc(uint8_t* message, size_t payload_size)
{
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < payload_size; pos++) {
    crc = update_crc(crc, message[pos]);
  }
  message[payload_size] = LOBYTE(crc);
  message[payload_size + 1] = HIBYTE(crc);
}
