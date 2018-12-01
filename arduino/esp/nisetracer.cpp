#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <esp_wifi.h>
#endif

#include "globals.h"
#include "crc.h"

inline static uint16_t HIWORD(uint32_t dword) { return (uint16_t)((dword & 0xffff0000) >> 16); }
inline static uint16_t LOWORD(uint32_t dword) { return (uint16_t)(dword & 0xffff); }

void setup_nisetracer()
{
    Serial.println("Setup nisetracer");
    modbus.enter_slave();

#ifdef ARDUINO_ARCH_ESP8266
    Serial.println("Entering modem sleep(MODEM_SLEEP_T)...");
    wifi_set_sleep_type(MODEM_SLEEP_T);
#elif defined ARDUINO_ARCH_ESP32
    Serial.println("Entering modem sleep(WIFI_PS_MAX_MODEM)...");
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM) == ESP_OK;
#endif
}

static uint16_t get_data(uint16_t addr)
{
  if (addr == 0x3100) {
    return 1234;
  } else if (addr == 0x3101) {
    return 100;
  } else if (addr == 0x3102) {
    return LOWORD(1234);
  } else if (addr == 0x3103) {
    return HIWORD(1234);
  } else if (addr == 0x3104) {
    return 1233;
  } else if (addr == 0x3105) {
    return 100;
  }
  // else
  return 0;
}

void loop_nisetracer()
{
  uint8_t modbus_message[MAX_MODBUS_MESSAGE_LENGTH];
  const uint8_t ptn_device_info[] = { 0x01, 0x2b, 0x0e, 0x01/*basic info*/,0x00 };
  int message_size = modbus.receive_modbus_message(modbus_message);
  if (message_size < 1) return;
  //else
  if (message_size < 4) {
    Serial.println("modbus: Message too short");
    return;
  }
  // else
  if (message_size - 2 == sizeof(ptn_device_info) && memcmp(modbus_message, ptn_device_info, sizeof(ptn_device_info)) == 0) {
    Serial.println("Device info");
    uint8_t response[] = { 0x01, 0x2b, 0x0e, 0x01, 0x00, 0x00, 0x00,
      0x03/*3 objects*/,
      0x00/*VendorName*/, 8, 'W', 'B', 'R', 'X', 'C', 'O', 'R', 'P',
      0x01/*ProductCode*/, 10, 'N', 'i', 's', 'e', 'T', 'r', 'a', 'c', 'e', 'r',
      0x02/*MajorMinorRevision*/, 3, '1', '.', '0',
      0, 0};
    put_crc(response, sizeof(response) - 2);

    modbus.send_modbus_message(response, sizeof(response));
  } else if (modbus_message[0] == 0x01 && (modbus_message[1] == 0x01 || modbus_message[1] == 0x03 || modbus_message[1] == 0x04)) {
    uint8_t func = modbus_message[1];
    if (func == 0x01) Serial.println("Get coil status");
    else if (func == 0x03) Serial.println("Get holding register");
    else if (func == 0x04) Serial.println("Get input register");

    if (message_size < 6 + 2) {
      Serial.println("modbus: Message too short");
      return;
    }
    // else
    uint16_t addr = MKWORD(modbus_message[2], modbus_message[3]);
    uint8_t num = MKWORD(modbus_message[4], modbus_message[5]);

    uint8_t response[3 + num * 2 + 2];
    response[0] = 0x01;
    response[1] = func;
    response[2] = num * 2;
    for (int i = 0; i < num; i++) {
      uint16_t data = get_data(addr + i);
      response[3 + i * 2] = HIBYTE(data);
      response[3 + i * 2 + 1] = LOBYTE(data);
    }
    put_crc(response, sizeof(response) - 2);

    modbus.send_modbus_message(response, sizeof(response));
  } else {
    Serial.println("Message received!");
  }
}
