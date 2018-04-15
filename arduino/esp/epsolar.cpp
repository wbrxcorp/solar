#include "globals.h"
#include "epsolar.h"

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

static bool receive_modbus_input_response(EPSOLAR_SERIAL_TYPE& RS485, uint8_t slave_id, uint8_t function_code, EPSolarTracerInputRegister& reg)
{
  uint8_t hdr[3];
  int nread = RS485.readBytes(hdr, sizeof(hdr));
  if (nread != sizeof(hdr)) {
    if (nread == 0) {
      Serial.println("modbus: response timeout");
    } else {
      Serial.printf("modbus: received header too short(expected=%u octets,actual=%d octets)", sizeof(hdr), nread);
      Serial.println();
    }
    return false;
  }
  //else
  if (hdr[0] != slave_id ||  hdr[1] != function_code) {
    Serial.println("modbus: response slave id or function code mismatch");
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
