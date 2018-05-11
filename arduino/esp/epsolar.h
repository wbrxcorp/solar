#ifndef __EPSOLAR_H_
#define __EPSOLAR_H_

#ifdef ARDUINO_ARCH_ESP8266
#include <SoftwareSerial.h>
#endif

#include "crc.h"

#define EPSOLAR_COMM_SPEED 115200
#define MIN_MESSAGE_INTERVAL 10

class EPSolarTracerDeviceInfo {
  String vendor_name;
  String product_code;
  String revision;
public:
  EPSolarTracerDeviceInfo() {
    ;
  }
  void set_value(uint8_t object_id, uint8_t object_length, const uint8_t* object_value) {
    if (object_id == 0x00/*VendorName*/) {
      vendor_name = "";
      for (int i = 0; i < object_length; i++) {
        vendor_name += (char)object_value[i];
      }
    } else if (object_id == 0x01/*ProductCode*/) {
      product_code = "";
      for (int i = 0; i < object_length; i++) {
        product_code += (char)object_value[i];
      }
    } else if (object_id == 0x02/*MajorMinorRevision*/) {
      revision = "";
      for (int i = 0; i < object_length; i++) {
        revision += (char)object_value[i];
      }
    };
  }
  const char* get_vendor_name() const { return vendor_name.c_str(); }
  const char* get_product_code() const { return product_code.c_str(); }
  const char* get_revision() const { return revision.c_str(); }
};

class EPSolarTracerInputRegister {
  uint8_t* _data;
  size_t _size;

  inline uint16_t MKWORD(uint8_t hi, uint8_t lo) const {
    return ((uint16_t)hi) << 8 | lo;
  }
  inline uint32_t MKDWORD(uint16_t hi, uint16_t lo) const {
    return ((uint32_t)hi) << 16 | lo;
  }
public:
  EPSolarTracerInputRegister() : _data(NULL), _size(0) {
  }
  EPSolarTracerInputRegister(const uint8_t* data, size_t size) {
    setData(data, size);
  }
  EPSolarTracerInputRegister(const EPSolarTracerInputRegister& other) {
    setData(other.data(), other.size());
  }
  ~EPSolarTracerInputRegister() {
    if (_data) delete []_data;
  }
  void setData(const uint8_t* data, size_t size) {
    if (_data && size > _size) {
      delete []_data;
      _data = NULL;
    }
    if (_data == NULL) {
      _data = new uint8_t[size];
    }
    memcpy(_data, data, size);
    _size = size;
  }
  size_t size() const { return _size; }
  const uint8_t* data() const { return _data; }

  uint16_t getWordValue(size_t offset) const {
    if (offset > _size - 2) return 0;
    return MKWORD(_data[offset], _data[offset + 1]);
  }

  int16_t getIntValue(size_t offset) const {
    return (int16_t)getWordValue(offset);
  }

  float getFloatValue(size_t offset) const {
    return (float)getIntValue(offset) / 100.0f;
  }

  double getDoubleValue(size_t offset) const {
    if (offset > _size - 4) return 0.0;
    return (double)MKDWORD(MKWORD(_data[offset + 2], _data[offset + 3]), MKWORD(_data[offset], _data[offset + 1])) / 100.0;
  }

  uint64_t getRTCValue(size_t offset) const {
    if (offset > _size - 6) return 0L;
    uint16_t r9013 = MKWORD(_data[offset], _data[offset + 1]);
    uint16_t r9014 = MKWORD(_data[offset + 2], _data[offset + 3]);
    uint16_t r9015 = MKWORD(_data[offset + 4], _data[offset + 5]);
    uint16_t year = 2000 + (r9015 >> 8);
    uint8_t month = r9015 & 0x00ff;
    uint8_t day = r9014 >> 8;
    uint8_t hour = r9014 & 0x00ff;
    uint8_t minute = r9013 >> 8;
    uint8_t second = r9013 & 0x00ff;
    return year * 10000000000LL + month * 100000000LL + day * 1000000L
      + hour * 10000L + minute * 100 + second;
  }

  bool getBoolValue(uint16_t bit_index) const { // for coils
    uint8_t byte_index = bit_index / 8;
    if (byte_index >= _size) return false;
    bit_index = bit_index % 8;
    return (_data[byte_index] >> bit_index) > 0;
  }
};

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_AVR_MEGA2560)
typedef HardwareSerial EPSOLAR_SERIAL_TYPE;
#elif ARDUINO_ARCH_ESP8266
typedef SoftwareSerial EPSOLAR_SERIAL_TYPE;
#endif

class EPSolar {
  EPSOLAR_SERIAL_TYPE* RS485;
  int rtsPin;
  unsigned long last_message;
public:
  EPSolar() : RS485(NULL), last_message(0L) {;}

  void begin(EPSOLAR_SERIAL_TYPE* _RS485, int _rtsPin)
  {
    RS485 = _RS485;
    rtsPin = _rtsPin;
    last_message = 0L;
    pinMode(rtsPin, OUTPUT);
  }

  void send_modbus_message(const uint8_t* message, size_t size)
  {
    unsigned long current_time = millis();
    if (current_time - last_message < MIN_MESSAGE_INTERVAL) {
      delay(MIN_MESSAGE_INTERVAL - (current_time - last_message));
    }
    EPSOLAR_SERIAL_TYPE& RS485 = *(this->RS485);
    while(RS485.available()) RS485.read();
    digitalWrite(rtsPin,HIGH);
    //delayMicroseconds(500);
    RS485.write(message, size);
    RS485.flush();
    delayMicroseconds(500);
    digitalWrite(rtsPin,LOW);
    last_message = current_time;
  }

  // http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  bool get_device_info(EPSolarTracerDeviceInfo& info, int max_retry = 5);

  bool get_register(uint16_t addr, uint8_t num, EPSolarTracerInputRegister& reg, int max_retry = 5);

  bool put_register(uint16_t addr, uint16_t data);

  bool put_registers(uint16_t addr, uint16_t* data, uint16_t num);

  void load_on(bool on)
  {
    put_register(0x0002/*manual load control*/, on? (uint16_t)0xff00:(uint16_t)0x0000);
  }

  bool set_rtc(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, uint16_t second)
  {
    if (year > 99 || month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) return false;
    uint16_t data[3];
    data[0/*0x9013*/] = minute << 8 | second;
    data[1/*0x9014*/] = day << 8 | hour;
    data[2/*0x9015*/] = year << 8 | month;
    return put_registers(0x9013/*Real Time Clock*/, data, 3);
  }
};

#endif // __EPSOLAR_H_
