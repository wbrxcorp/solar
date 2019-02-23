#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <SPIFFS.h>
#endif
#include <FS.h>

#include "globals.h"

#define BME280_I2C_ADDRESS 0x76 // GYBMEP
#define CCS811_I2C_ADDRESS 0x5A

static bool ccs811_present = false;

void early_setup_thermometer()
{
  WiFi.mode(WIFI_OFF);
  tft.setRotation(config.tft_rotation);
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
void load_background_image()
{
  Serial.println("Loading background image from SPIFFS...");
  SPIFFS.begin();
  File f = SPIFFS.open("/background.bmp", "r");
  bool loaded = false;
  if (f) {
    loaded = tft.showBitmapFile(f);
    f.close();
  }

  if (loaded) {
    Serial.println("background.bmp loaded");
  } else {
    Serial.println("background.bmp not loaded");
    tft.fillScreen(TFT_WHITE);
  }
  SPIFFS.end();
}


void setup_thermometer()
{
  load_background_image();

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

  if (ccs.begin(CCS811_I2C_ADDRESS)) {
    ccs811_present = true;
    while(!ccs.available());
    float temp = ccs.calculateTemperature();
    ccs.setTempOffset(temp - 25.0);
  } else {
    Serial.println("Could not find a CCS811 sensor.");
  }
}

void thermometer_print_values(float temperature, float humidity, uint16_t pressure)
{
  char buf[16];
  sprintf(buf, "%2.1f", temperature);
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
  sprintf(buf, "%2.1f", humidity);
  tft.setTextSize(3);
  tft.setCursor(13, 122);
  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.print(buf);

  // pressure
  sprintf(buf, "%4d", pressure);
  tft.setTextSize(3);
  tft.setCursor(10, 155);
  tft.setTextColor(TFT_GREEN, TFT_WHITE);
  tft.print(buf);

  if (ccs811_present && ccs.available()) {
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
      // eCO2
      sprintf(buf, "%4d", (int)ccs.geteCO2());
      tft.setCursor(10, 188);
      tft.setTextColor(TFT_ORANGE, TFT_WHITE);
      tft.print(buf);

      // TVOC
      Serial.print(" ppm, TVOC: ");
      float TVOC = ccs.getTVOC();
      Serial.print(TVOC);

      // CCS811's temperature
      Serial.print(" ppb   Temp:");
      Serial.println(temp);
    }
  }

}

void loop_thermometer()
{
  thermometer_print_values(bme.readTemperature(), bme.readHumidity(), (uint16_t)(bme.readPressure() / 100.0F));
  delay(1000);
}
