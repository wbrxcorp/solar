#ifndef __EPSOLAR_H_
#define __EPSOLAR_H_

#include "crc.h"

#define EPSOLAR_COMM_SPEED 115200

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

class EPSolar {
  Stream* RS485;
  int rtsPin;
public:
  EPSolar() : RS485(NULL) {;}

  void begin(Stream* _RS485, int _rtsPin)
  {
    RS485 = _RS485;
    rtsPin = _rtsPin;
    pinMode(rtsPin, OUTPUT);
  }

  void send_modbus_message(const uint8_t* message, size_t size)
  {
    Stream& RS485 = *(this->RS485);
    while(RS485.available()) RS485.read();
    digitalWrite(rtsPin,HIGH);
    //delayMicroseconds(500);
    RS485.write(message, size);
    RS485.flush();
    delayMicroseconds(500);
    digitalWrite(rtsPin,LOW);
  }

  // http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
  bool get_device_info(EPSolarTracerDeviceInfo& info, int max_retry = 5)
  {
    byte message[] = {0x01, 0x2b, 0x0e, 0x01/*basic info*/,0x00, 0x00, 0x00 };
    put_crc(message, sizeof(message) - 2);

    send_modbus_message(message, sizeof(message));

    byte hdr[8];
    int retry_count = 0;
    while (retry_count < max_retry) {
      Stream& RS485 = *(this->RS485);
      if (RS485.readBytes(hdr, sizeof(hdr)) && memcmp(hdr, message, 4) == 0) {
        uint16_t crc = 0xffff;
        for (int i = 0; i < sizeof(hdr); i++) crc = update_crc(crc, hdr[i]);

        uint8_t num_objects = hdr[7];
        for (int i = 0; i < num_objects; i++) {
          uint8_t object_hdr[2];
          if (RS485.readBytes(object_hdr, sizeof(object_hdr)) != sizeof(object_hdr)) break;
          crc = update_crc(update_crc(crc, object_hdr[0]), object_hdr[1]);
          uint8_t object_value[object_hdr[1]];
          if (RS485.readBytes(object_value, sizeof(object_value)) != sizeof(object_value)) break;
          for (int i = 0; i < sizeof(object_value); i++) crc = update_crc(crc, object_value[i]);
          info.set_value(object_hdr[0], object_hdr[1], object_value);
        }
        // crc check
        uint8_t rx_crc[2] = {0, 0};
        if (RS485.readBytes(rx_crc, sizeof(rx_crc)) == sizeof(rx_crc) && !RS485.available() && rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc)) return true;
      }
      // else
      while (RS485.available()) RS485.read(); // discard remaining bytes
      Serial.println("Retrying...");
      delay(200);
      retry_count++;
    }
  }

  void print_bytes(const uint8_t* bytes, size_t size)
  {
    for (int i = 0; i < size; i++) {
      char buf[8];
      sprintf(buf, "%02x ", bytes[i]);
      Serial.print(buf);
    }
    Serial.println();
  }

  bool get_register(uint16_t addr, uint8_t num, EPSolarTracerInputRegister& reg, int max_retry = 10)
  {
    uint8_t function_code = 0x04; // Read Input Register
    if (addr >= 0x9000 && addr < 0x9100) function_code = 0x03; // Read Holding Register
    if (addr < 0x2000) function_code = 0x01; // Read Coil Status

    uint8_t message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), 0x00, num, 0x00, 0x00 };
    put_crc(message, sizeof(message) - 2);
    //print_bytes(message, sizeof(message));

    for (int i = 0; i < max_retry; i++) {
      send_modbus_message(message, sizeof(message));

      uint8_t hdr[3];
      Stream& RS485 = *(this->RS485);
      if (RS485.readBytes(hdr, sizeof(hdr)) == sizeof(hdr)) {
        //Serial.print("hdr received: ");
        //print_bytes(hdr, 3);
        if (hdr[0] == message[0] && hdr[1] == message[1]) { // check function code and slave address
          size_t data_size = (size_t)hdr[2];
          if (data_size < 128) { // too big data
            uint8_t buf[data_size];
            if (RS485.readBytes(buf, sizeof(buf)) == sizeof(buf)) {
              uint8_t rx_crc[2] = {0, 0};
              if (RS485.readBytes(rx_crc, sizeof(rx_crc)) == sizeof(rx_crc) && !RS485.available()) {
                // crc check
                uint16_t crc = update_crc(update_crc(update_crc(0xFFFF,hdr[0]), hdr[1]), hdr[2]);
                for (int i = 0; i < sizeof(buf); i++) crc = update_crc(crc, buf[i]);
                if (rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc)) {
                  reg.setData(buf, data_size);
                  return true;
                } else {
                  Serial.println("CRC doesn't match");
                }
              }
            }
          }
        }
      }
      // else
      int available = RS485.available();
      if (available) {
        uint8_t buf[available];
        int size = RS485.readBytes(buf, sizeof(buf));
        Serial.print("Remains: ");
        print_bytes(buf, size);
      }
      Serial.println("Retrying...");
      delay(200);
    }
    return false;
  }
  bool put_register(uint16_t addr, uint16_t data)
  {
    uint8_t function_code = 0x06; // Preset Single Register(06)
    if (addr < 0x2000) function_code = 0x05; // Force Single Coil
    byte message[] = {0x01, function_code, HIBYTE(addr), LOBYTE(addr), HIBYTE(data), LOBYTE(data), 0x00, 0x00 };
    put_crc(message, sizeof(message) - 2);

    send_modbus_message(message, sizeof(message));
    delay(50);
    Stream& RS485 = *(this->RS485);
    while (RS485.available()) RS485.read(); // simply discard response(TODO: check the response)
    return true;
  }

  bool put_registers(uint16_t addr, uint16_t* data, uint16_t num)
  {
    uint8_t data_size_in_bytes = (uint8_t)(sizeof(*data) * num);
    size_t message_size = 9/*slave address, func code, start addr(H+L), num(H+L), length in bytes, ... , crc(L/H)*/ + data_size_in_bytes;
    byte message[message_size];
    message[0] = 0x01;
    message[1] = 0x10;
    message[2] = HIBYTE(addr);
    message[3] = LOBYTE(addr);
    message[4] = HIBYTE(num);
    message[5] = LOBYTE(num);
    message[6] = data_size_in_bytes;
    for (int i = 0; i < num; i++) {
      message[7 + i * 2] = HIBYTE(data[i]);
      message[8 + i * 2] = LOBYTE(data[i]);
    }
    put_crc(message, message_size - 2);

    //print_bytes(message, sizeof(message));
    send_modbus_message(message, sizeof(message));
    delay(50);
    Stream& RS485 = *(this->RS485);
    while (RS485.available()) RS485.read(); // simply discard response(TODO: check the response)
    return true;
  }

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
