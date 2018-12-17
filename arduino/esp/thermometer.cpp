#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <SPIFFS.h>
#endif
#include <FS.h>

#include <Adafruit_BME280.h>
#include "globals.h"

#define BME280_I2C_ADDRESS 0x76 // GYBMEP

void early_setup_thermometer()
{
  WiFi.mode(WIFI_OFF);
#if defined(ARDUINO_ARCH_ESP8266)
  tft.begin((int8_t)0, (int8_t)15);
#elif defined(ARDUINO_ARCH_ESP32)
  tft.begin((int8_t)5, (int8_t)0);
#endif
}

// for 4M1M esp8266
// mkspiffs -c . -b 8192 -p 256 -s 0xfb000 /tmp/esp8266.spiffs
// esptool.py --baud 460800 --port /dev/ttyUSB0 write_flash 0x300000 /tmp/esp8266.spiffs
//
// mkspiffs -c . -b 4096 -p 256 -s 0x16f000 /tmp/esp32.spiffs
// esptool.py --chip esp32 --baud 460800 --port /dev/ttyUSB0 write_flash 0x291000 /tmp/esp32.spiffs
void setup_thermometer()
{
  SPIFFS.begin();
  File f = SPIFFS.open("/background.bmp", "r");
  if (f) {
    if (tft.showBitmapFile(f)) {
      Serial.println("background.bmp loaded");
    }
    f.close();
  } else {
    Serial.println("No background.bmp");
    tft.fillScreen(TFT_WHITE);
  }
  SPIFFS.end();

  if (!bme.begin(BME280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.println("SENSOR NOT FOUND");
    tft.println("REBOOT AFTER 5SEC");
#if defined(ARDUINO_ARCH_ESP8266)
    ESP.deepSleep(5 * 1000L * 1000L , WAKE_RF_DEFAULT);
#elif defined(ARDUINO_ARCH_ESP32)
    esp_sleep_enable_timer_wakeup(5 * 1000L * 1000L);
    esp_deep_sleep_start();
#endif
    delay(1000);
  }
}

void loop_thermometer()
{
  char buf[16];
  sprintf(buf, "%2.1f", bme.readTemperature());
  char* decimal_part = strchr(buf, '.');

  if (decimal_part) {
    // decimal part first
    tft.setTextColor(TFT_RED, TFT_WHITE);
    tft.setTextSize(3);
    tft.setCursor(108, 20);
    tft.print(decimal_part);

    *decimal_part = '\0';
  }

  tft.setTextSize(8);
  tft.setCursor(10, 16);
  tft.print(buf);

  // humidity
  sprintf(buf, "%2.1f", bme.readHumidity());
  tft.setTextSize(3);
  tft.setCursor(13, 122);
  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.print(buf);

  // pressure
  sprintf(buf, "%4d", (int)(bme.readPressure() / 100.0F));
  tft.setTextSize(3);
  tft.setCursor(10, 155);
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
  tft.print(buf);

  delay(1000);
}
