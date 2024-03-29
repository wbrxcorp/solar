#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <map>
#include "freertos/FreeRTOS.h"

namespace modbus {

static const int MAX_MESSAGE_LENGTH = 255;

inline uint16_t MKWORD(uint8_t hi, uint8_t lo)
{
  return ((uint16_t)hi) << 8 | lo;
}

inline uint32_t MKDWORD(uint16_t hi, uint16_t lo)
{
  return ((uint32_t)hi) << 16 | lo;
}

class Driver {
public:
  virtual void send_message(const uint8_t* message, size_t size) = 0;
  virtual int receive_message(uint8_t* modbus_message) = 0;
};

class DeviceInfo : public std::map<uint8_t,std::string> {
public:
    const char* get_vendor_name() const { 
        auto i = this->find(0x00/*VendorName*/);
        return i != this->end()? i->second.c_str() : NULL;
    }
    const char* get_product_code() const {
        auto i = this->find(0x01/*ProductCode*/);
        return i != this->end()? i->second.c_str() : NULL; 
    }
    const char* get_revision() const { 
        auto i = this->find(0x02/*MajorMinorRevision*/);
        return i != this->end()? i->second.c_str() : NULL;
    }
};

class ReadQueryResponse {
protected:
  uint8_t* data;
  size_t size;
public:
  ReadQueryResponse() : data(NULL), size(0) { ; }
  ~ReadQueryResponse() { if (data) delete []data; }
  void setData(const uint8_t* _data, size_t _size);
};

// function code 1, 2
class StatusReadQueryResponse : public ReadQueryResponse {
public:
  uint8_t getByte(size_t idx) const;
  bool getBool(uint16_t bit_index) const { // for coils
    uint8_t byte_index = bit_index / 8;
    if (byte_index >= size) return false;
    bit_index = bit_index % 8;
    return (data[byte_index] >> bit_index) > 0;
  }
};

// function code 3, 4
class RegisterReadQueryResponse : public ReadQueryResponse {
public:
  uint16_t getWord(size_t idx) const;
};

void init(Driver& driver);
bool read_query(uint8_t slave_id, uint8_t function_code/*1, 2, 3, 4*/,
    uint16_t addr, uint16_t num, ReadQueryResponse& response, int max_retry = 5);
bool get_basic_device_info(std::map<uint8_t,std::string>& info, int max_retry = 5);
bool write_query(uint8_t slave_id, uint8_t function_code/*5(force single coil), 6(preset single register)*/,
   uint16_t addr, uint16_t data, int max_retry = 5);
bool write_query(uint8_t slave_id, uint16_t addr, const uint16_t* data, uint16_t num, int max_retry = 5); // 16(Preset Multiple Registers)

} // namespace modbus

#endif // __MODBUS_H__