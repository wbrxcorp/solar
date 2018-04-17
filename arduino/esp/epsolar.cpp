#include "globals.h"
#include "epsolar.h"

static const char* exception_codes[] = {
  "UNKNOWN(0x00)",
  "ILLEGAL FUNCTION(0x01)",
  "ILLEGAL DATA ADDRESS(0x02)",
  "ILLEGAL DATA VALUE(0x03)",
  "SLAVE DEVICE FAILURE(0x04)",
  "ACKNOWLEDGE(0x05)",
  "SLAVE DEVICE BUSY(0x06)",
  "UNKNOWN(0x07)",
  "MEMORY PARITY ERROR(0x08)",
  "UNKNOWN(0x09)",
  "GATEWAY PATH UNAVAILABLE(0x0a)",
  "GATEWAY TARGET DEVICE FAILED TO RESPOND(0x0b)"
};

static void print_bytes(const uint8_t* bytes, size_t size)
{
  for (int i = 0; i < size; i++) {
    Serial.printf("%02x ", bytes[i]);
  }
  Serial.println();
}

static bool receive_modbus_device_info_response(EPSOLAR_SERIAL_TYPE& RS485, uint8_t slave_id, EPSolarTracerDeviceInfo& info)
{
  uint8_t hdr[8];
  if (RS485.readBytes(hdr, sizeof(hdr)) != sizeof(hdr)) return false;
  // else
  if (hdr[0] != slave_id || hdr[1] != 0x2b || hdr[2] != 0x0e || hdr[3] != 0x01) return false;
  // else
  uint16_t crc = 0xffff;
  for (int i = 0; i < sizeof(hdr); i++) crc = update_crc(crc, hdr[i]);
  uint8_t num_objects = hdr[7];
  for (int i = 0; i < num_objects; i++) {
    uint8_t object_hdr[2];
    if (RS485.readBytes(object_hdr, sizeof(object_hdr)) != sizeof(object_hdr)) return false;
    crc = update_crc(update_crc(crc, object_hdr[0]), object_hdr[1]);
    uint8_t object_value[object_hdr[1]];
    if (RS485.readBytes(object_value, sizeof(object_value)) != sizeof(object_value)) return false;
    for (int i = 0; i < sizeof(object_value); i++) crc = update_crc(crc, object_value[i]);
    info.set_value(object_hdr[0], object_hdr[1], object_value);
  }
  // crc check
  uint8_t rx_crc[2] = {0, 0};
  if (RS485.readBytes(rx_crc, sizeof(rx_crc)) != sizeof(rx_crc)) return false;
  // else
  return (rx_crc[0] == LOBYTE(crc) && rx_crc[1] == HIBYTE(crc));
}

bool EPSolar::get_device_info(EPSolarTracerDeviceInfo& info, int max_retry/* = 5*/)
{
  uint8_t slave_id = 0x01; // Read Input Register
  uint8_t message[] = {slave_id, 0x2b, 0x0e, 0x01/*basic info*/,0x00, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  for (int i = 0; i < max_retry; i++) {
    send_modbus_message(message, sizeof(message));
    if (receive_modbus_device_info_response(*(this->RS485), slave_id, info)) {
      if (i > 0) Serial.println("Retry successful.");
      return true;
    }
    //else
    Serial.println("Retrying...");
  }

  return false;
}

static bool read_response_bytes(EPSOLAR_SERIAL_TYPE& RS485, uint8_t* buf, size_t expected_size)
{
  int nread = RS485.readBytes(buf, expected_size);
  if (nread != expected_size) {
    if (nread == 0) {
      Serial.println("modbus: response timeout");
    } else {
      Serial.printf("modbus: received data too short(expected=%u octets,actual=%d octets)", expected_size, nread);
      Serial.println();
    }
    return false;
  }
  // else
  return true;
}

static bool receive_modbus_input_response(EPSOLAR_SERIAL_TYPE& RS485, uint8_t slave_id, uint8_t function_code, EPSolarTracerInputRegister& reg)
{
  uint8_t hdr[3];
  if (!read_response_bytes(RS485, hdr, sizeof(hdr))) return false;
  if (hdr[0] != slave_id) {
    Serial.println("modbus: response slave id mismatch");
    return false;
  }
  if (hdr[1] == 0x83/*Error*/) {
    Serial.printf("modbus: slave returned error response(0x83). %s", exception_codes[hdr[2]]);
    Serial.println();
    uint8_t crc[2];
    if (read_response_bytes(RS485, crc, sizeof(crc))) {
      // TODO: check CRC
    }
    return false;
  }
  //else
  if (hdr[1] != function_code) {
    Serial.printf("modbus: response function code mismatch. expected=0x%02x,actual=0x%02x", (int)function_code, (int)hdr[1]);
    Serial.println();
    return false;
  }
  // else
  size_t data_size = (size_t)hdr[2];
  uint8_t buf[data_size];
  if (RS485.readBytes(buf, sizeof(buf)) != sizeof(buf)) {
    Serial.printf("modbus: received response payload too short(expected=%u octets)", sizeof(buf));
    Serial.println();
    return false;
  }
  // else
  uint8_t rx_crc[2] = {0, 0};
  if (RS485.readBytes(rx_crc, sizeof(rx_crc)) != sizeof(rx_crc)) {
    Serial.println("modbus: received crc too short(expected=2 octets)");
    return false;
  }
  // else
  uint16_t crc = update_crc(update_crc(update_crc(0xFFFF,hdr[0]), hdr[1]), hdr[2]);
  for (int i = 0; i < sizeof(buf); i++) crc = update_crc(crc, buf[i]);
  if (rx_crc[0] != LOBYTE(crc) || rx_crc[1] != HIBYTE(crc)) {
    Serial.println("modbus: CRC mismatch");
    return false;
  }

  reg.setData(buf, data_size);
  return true;
}

bool EPSolar::get_register(uint16_t addr, uint8_t num, EPSolarTracerInputRegister& reg, int max_retry/* = 10*/)
{
  uint8_t slave_id = 0x01, function_code = 0x04; // Read Input Register
  if (addr >= 0x9000 && addr < 0x9100) function_code = 0x03; // Read Holding Register
  if (addr < 0x2000) function_code = 0x01; // Read Coil Status

  uint8_t message[] = {slave_id, function_code, HIBYTE(addr), LOBYTE(addr), 0x00, num, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  for (int i = 0; i < max_retry; i++) {
    send_modbus_message(message, sizeof(message));
    if (debug_mode) {
      Serial.print("modbus sent: ");
      print_bytes(message, sizeof(message));
    }
    if (receive_modbus_input_response(*(this->RS485), slave_id, function_code, reg)) {
      if (i > 0) Serial.println("Retry successful.");
      return true;
    }
    // else
    Serial.println("Retrying...");
  }

  return false;
}

static bool receive_modbus_output_response(EPSOLAR_SERIAL_TYPE& RS485, uint8_t slave_id, uint8_t function_code)
{
  uint8_t response[3];
  if (!read_response_bytes(RS485, response, 2)) return false;
  //else
  if (response[0] != slave_id) {
    Serial.println("modbus: response slave id mismatch");
    return false;
  }
  if (response[1] == 0x83/*Error*/) {
    if (!read_response_bytes(RS485, response + 2, 1)) return false;
    Serial.printf("modbus: slave returned error response(0x83). %s", exception_codes[response[2]]);
    Serial.println();
    uint8_t crc[2];
    if (read_response_bytes(RS485, crc, sizeof(crc))) {
      // TODO: check CRC
    }
    return false;
  }
  //else
  if (response[1] != function_code) {
    Serial.printf("modbus: response function code mismatch. expected=0x%02x,actual=0x%02x", (int)function_code, (int)response[1]);
    Serial.println();
    return false;
  }
  //else
  return true;
}


bool EPSolar::put_register(uint16_t addr, uint16_t data)
{
  uint8_t slave_id = 0x01, function_code = 0x06; // Preset Single Register(06)
  if (addr < 0x2000) function_code = 0x05; // Force Single Coil
  byte message[] = {slave_id, function_code, HIBYTE(addr), LOBYTE(addr), HIBYTE(data), LOBYTE(data), 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  send_modbus_message(message, sizeof(message));
  delay(50);

  EPSOLAR_SERIAL_TYPE& RS485 = *(this->RS485);
  if (!receive_modbus_output_response(RS485, slave_id, function_code)) return false;
  //else
  while (RS485.available()) RS485.read(); // drop remaining
  return true;
}

bool EPSolar::put_registers(uint16_t addr, uint16_t* data, uint16_t num)
{
  uint8_t data_size_in_bytes = (uint8_t)(sizeof(*data) * num);
  size_t message_size = 9/*slave address, func code, start addr(H+L), num(H+L), length in bytes, ... , crc(L/H)*/ + data_size_in_bytes;
  uint8_t message[message_size];
  uint8_t slave_id = 0x01, function_code = 0x10; // Preset Multiple Registers(16, 0x10)
  message[0] = slave_id;
  message[1] = function_code;
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
  EPSOLAR_SERIAL_TYPE& RS485 = *(this->RS485);
  if (!receive_modbus_output_response(RS485, slave_id, function_code)) return false;
  //else
  while (RS485.available()) RS485.read(); // drop remaining
  return true;
}
