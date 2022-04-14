#include "modbus.h"

#include <memory.h>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char* TAG = "modbus";

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

static SemaphoreHandle_t lock;
#define LOCK          xSemaphoreTake(lock, portMAX_DELAY)
#define UNLOCK        xSemaphoreGive(lock)

inline static uint8_t HIBYTE(uint16_t word) { return (uint8_t)((word & 0xff00) >> 8); }
inline static uint8_t LOBYTE(uint16_t word) { return (uint8_t)(word & 0xff); }

static uint16_t update_crc(uint16_t crc, uint8_t val)
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

static void put_crc(uint8_t* message, size_t payload_size)
{
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < payload_size; pos++) {
    crc = update_crc(crc, message[pos]);
  }
  message[payload_size] = LOBYTE(crc);
  message[payload_size + 1] = HIBYTE(crc);
}

namespace modbus {

static Driver* driver = NULL;

void ReadQueryResponse::setData(const uint8_t* _data, size_t _size) {
  if (data) delete []data;
  size = _size;
  data = new uint8_t[size];
  memcpy(data, _data, size);
}

uint16_t RegisterReadQueryResponse::getWord(size_t idx) const {
  auto offset = idx * sizeof(uint16_t);
  if (offset > size - 2) return 0;
  return MKWORD(data[offset], data[offset + 1]);
}

void init(Driver& _driver)
{
  driver = &_driver;
  lock = xSemaphoreCreateMutex();
  if (!lock) abort();
}

void send_message(const uint8_t* message, size_t size)
{
  return driver->send_message(message, size);
}

int receive_message(uint8_t* modbus_message)
{
  auto message_size = driver->receive_message(modbus_message);

  if (message_size == 0) {
    ESP_LOGE(TAG, "Response timeout(No device connected?)");
    return -1;
  }

  if (message_size < 4) {
    ESP_LOGE(TAG, "Response message too short(expected at least 4 octets, received %d octets)", message_size);
    return -1;
  }

  uint16_t crc = 0xffff;
  for (int i = 0; i < message_size - 2; i++) crc = update_crc(crc, modbus_message[i]);
  if (modbus_message[message_size - 2] != LOBYTE(crc) || modbus_message[message_size - 1] != HIBYTE(crc)) {
    std::string message_hex;
    char buf[4];
    for (int i = 0; i < message_size; i++) {
      sprintf(buf, "%02x ", modbus_message[i]);
      message_hex += buf;
    }
    ESP_LOGE(TAG, "modbus: CRC mismatch. Message=[ %s ](%d bytes)", message_hex.c_str(), message_size);
    return -1;
  }

  return message_size;
}

bool read_query(uint8_t slave_id, uint8_t function_code/*1, 3, 4*/, 
  uint16_t addr, uint16_t num, ReadQueryResponse& response, int max_retry)
{
  uint8_t message[] = {slave_id, function_code, HIBYTE(addr), LOBYTE(addr), HIBYTE(num), 
    LOBYTE(num), 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);
  bool rst = false;

  for (int i = 0; i < max_retry; i++) {
    LOCK;
    send_message(message, sizeof(message));
    uint8_t data[MAX_MESSAGE_LENGTH];
    int message_size = receive_message(data);
    UNLOCK;
    if (message_size < 0) continue;
    if (message_size < 3/*hdr*/ + 2/*crc*/) {
      ESP_LOGE(TAG, "Response message too short(%d octets)", message_size);
      continue;
    }
    if (data[0] != slave_id) {
      ESP_LOGE(TAG, "Rresponse slave id mismatch. expected=0x%02x,actual=0x%02x.",
        (int)slave_id, (int)data[0]);
      continue;
    }
    if (data[1] == 0x83/*Error*/) {
      ESP_LOGE(TAG,"Slave returned error response(0x83). %s",
        exception_codes[data[2]]);
      continue;
    }
    if (data[1] != function_code) {
      ESP_LOGE(TAG,"Response function code mismatch. expected=0x%02x,actual=0x%02x.",
        (int)function_code, (int)data[1]);
    }
    size_t data_size = (size_t)data[2];
    if (data_size + 3/*hdr*/ + 2/*crc*/ > message_size) {
      ESP_LOGE(TAG,"Received response payload too short(expected=%d octets",
        data_size);
      continue;
    }
    //else
    response.setData(data + 3/*hdr*/, data_size);
    rst = true;
    break;
  }

  return rst;
}

bool write_query(uint8_t slave_id, uint8_t function_code/*5, 6*/,
  uint16_t addr, uint16_t data)
{
#if 0
  uint8_t slave_id = 0x01, function_code = 0x06; // Preset Single Register(06)
  if (addr < 0x2000) function_code = 0x05; // Force Single Coil
  byte message[] = {slave_id, function_code, HIBYTE(addr), LOBYTE(addr), HIBYTE(data), LOBYTE(data), 0x00, 0x00 };
  put_crc(message, sizeof(message) - 2);

  modbus.send_modbus_message(message, sizeof(message));

  if (!receive_modbus_output_response(slave_id, function_code)) return false;
  //else
#endif
  return true;
}

bool write_query(uint8_t slave_id, uint8_t function_code/*0x10*/,
  uint16_t addr, const uint16_t* data, uint16_t num)
{
#if 0
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
#endif
  return true;
}

} // namespace modbus