#include "globals.h"
#include "epsolar.h"
#include "crc.h"

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
  char buf[4];
  for (int i = 0; i < size; i++) {
    sprintf(buf, "%02x ", bytes[i]);
    Serial.print(buf);
  }
  Serial.println();
}

bool EPSolar::receive_modbus_device_info_response(uint8_t slave_id, EPSolarTracerDeviceInfo& info)
{
  uint8_t response[MAX_MODBUS_MESSAGE_LENGTH];
  int message_size = modbus.receive_modbus_message(response);
  if (message_size < 8 + 2/*hdr size + crc*/) return false;

  if (response[0] != slave_id) {
    Serial.println("modbus: Slave id mismatch.");
    return false;
  }

  if (response[1] != 0x2b) {
    Serial.println("modbus: Function code mismatch.(expected 0x2b)");
    return false;
  }

  if (response[2] != 0x0e || response[3] != 0x01) {
    Serial.println("modbus: Field mismatch.(expected 0x0e 0x01 = basic info)");
    return false;
  }

  // else
  uint8_t num_objects = response[7];
  const uint8_t* pt = response + 8;
  for (int i = 0; i < num_objects; i++) {
    if (pt - response + 2 >= message_size) {
      Serial.println("modbus: Device info response is too short");
      return false;
    }
    uint8_t object_id = pt[0];
    uint8_t object_length = pt[1];
    const uint8_t* object_value = pt + 2;

    if (pt - response + 2 + object_length >= message_size) {
      Serial.println("modbus: Device info response is too short");
      return false;
    }

    info.set_value(object_id, object_length, object_value);
    pt += 2 + object_length;
  }
  return true;
}

bool EPSolar::get_device_info(EPSolarTracerDeviceInfo& info, int max_retry/* = 5*/)
{
  uint8_t slave_id = 0x01; // Read Input Register
  uint8_t message[] = {slave_id, 0x2b, 0x0e, 0x01/*basic info*/,0x00, 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  for (int i = 0; i < max_retry; i++) {
    modbus.send_modbus_message(message, sizeof(message));
    if (receive_modbus_device_info_response(slave_id, info)) {
      if (i > 0) Serial.println("Retry successful.");
      return true;
    }
    //else
    Serial.println("Retrying...");
  }

  return false;
}

bool EPSolar::receive_modbus_input_response(uint8_t slave_id, uint8_t function_code, EPSolarTracerInputRegister& reg)
{
  uint8_t response[MAX_MODBUS_MESSAGE_LENGTH];
  int message_size = modbus.receive_modbus_message(response);
  if (message_size < 3/*hdr*/ + 2/*crc*/) return false;

  if (response[0] != slave_id) {
    char buf[3];
    Serial.print("modbus: response slave id mismatch. expected=0x");
    sprintf(buf, "%02x", (int)slave_id);
    Serial.print(buf);
    Serial.print(",actual=0x");
    sprintf(buf, "%02x", (int)response[0]);
    Serial.println(buf);
    return false;
  }
  if (response[1] == 0x83/*Error*/) {
    Serial.print("modbus: slave returned error response(0x83). ");
    Serial.println(exception_codes[response[2]]);
    Serial.println();
    return false;
  }
  //else
  if (response[1] != function_code) {
    char buf[3];
    Serial.print("modbus: response function code mismatch. expected=0x");
    sprintf(buf, "%02x", (int)function_code);
    Serial.print(buf);
    Serial.print(",actual=0x");
    sprintf(buf, "%02x", (int)response[1]);
    Serial.println(buf);
    return false;
  }
  // else
  size_t data_size = (size_t)response[2];
  if (data_size + 3/*hdr*/ + 2/*crc*/ > message_size) {
    Serial.print("modbus: received response payload too short(expected=");
    Serial.print(data_size);
    Serial.println(" octets)");
    return false;
  }
  // else
  reg.setData(response + 3/*hdr*/, data_size);
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
    modbus.send_modbus_message(message, sizeof(message));
    if (debug_mode) {
      Serial.print("modbus sent: ");
      print_bytes(message, sizeof(message));
    }
    if (receive_modbus_input_response(slave_id, function_code, reg)) {
      if (i > 0) Serial.println("Retry successful.");
      return true;
    }
    // else
    Serial.println("Retrying...");
  }

  return false;
}

bool EPSolar::receive_modbus_output_response(uint8_t slave_id, uint8_t function_code)
{
  uint8_t response[MAX_MODBUS_MESSAGE_LENGTH];
  int message_size = modbus.receive_modbus_message(response);
  if (message_size < 3/*hdr*/ + 2/*crc*/) return false;

  //else
  if (response[0] != slave_id) {
    char buf[3];
    Serial.print("modbus: response slave id mismatch. expected=0x");
    sprintf(buf, "%02x", (int)slave_id);
    Serial.print(buf);
    Serial.print(",actual=0x");
    sprintf(buf, "%02x", (int)response[0]);
    Serial.println(buf);
    return false;
  }
  // else
  if (response[1] == 0x83/*Error*/) {
    Serial.print("modbus: slave returned error response(0x83). ");
    Serial.println(exception_codes[response[2]]);
    return false;
  }
  //else
  if (response[1] != function_code) {
    char buf[3];
    Serial.print("modbus: response function code mismatch. expected=0x");
    sprintf(buf, "%02x", (int)function_code);
    Serial.print(buf);
    Serial.print(",actual=0x");
    sprintf(buf, "%02x", (int)response[1]);
    Serial.println(buf);
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

  modbus.send_modbus_message(message, sizeof(message));

  if (!receive_modbus_output_response(slave_id, function_code)) return false;
  //else
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
  modbus.send_modbus_message(message, sizeof(message));
  if (!receive_modbus_output_response(slave_id, function_code)) return false;
  //else
  return true;
}
