#ifndef __CRC_H_
#define __CRC_H_

#include <Arduino.h>

inline static uint8_t HIBYTE(uint16_t word) { return (uint8_t)((word & 0xff00) >> 8); }
inline static uint8_t LOBYTE(uint16_t word) { return (uint8_t)(word & 0xff); }

uint16_t update_crc(uint16_t crc, uint8_t val);
void put_crc(uint8_t* message, size_t payload_size);

#endif // __CRC_H_
