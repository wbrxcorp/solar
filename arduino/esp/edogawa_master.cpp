#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_sleep.h>
#endif

#include "globals.h"
#include "thermometer.h"

#define BME280_I2C_ADDRESS 0x76 // GYBMEP

#if defined(ARDUINO_ARCH_ESP8266)
  const uint8_t NUM_EDOGAWA_UNIT = 1;
  EdogawaUnit* edg = &edogawaUnit1;
  uint8_t swPin[NUM_EDOGAWA_UNIT] = { 14 };
  uint8_t ledPin[NUM_EDOGAWA_UNIT] = { 12 };
#elif defined(ARDUINO_ARCH_ESP32)
  const uint8_t NUM_EDOGAWA_UNIT = 5;
  EdogawaUnit edg[NUM_EDOGAWA_UNIT];
  uint8_t swPin[NUM_EDOGAWA_UNIT] = { 26, 17, 27, 12, 2 };
  uint8_t ledPin[NUM_EDOGAWA_UNIT] = { 25, 16, 14, 13, 4 };
#endif

unsigned long lastEdogawaUnitOperation = 0;
const uint16_t EDOGAWA_UNIT_OPERATION_INTERVAL_SECS = 300;

const float LOW_TEMPERATURE = 22.5;
const float HIGH_TEMPERATURE = 25.5;

uint8_t cnt = 0;

void early_setup_edogawa_master()
{
#if defined(ARDUINO_ARCH_ESP32)
  tft.begin((int8_t)5, (int8_t)0);
#endif
}

void setup_edogawa_master()
{
  if (!bme.begin(BME280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    display.println("SENSOR NOT FOUND");
    display.println("REBOOT AFTER 5SEC");
    display.display();

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
  // else
  Serial.println("Temperature sensor Initialized");

  for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
    edg[i].begin(swPin[i], ledPin[i]);
  }

  if (tft) {
    load_background_image(); // in thermometer.h

    for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
      tft.drawRect(20 * i, tft.height() - 19, 19, 19, TFT_RED);
    }
  }

  lastEdogawaUnitOperation = millis();
  delay(1000);
}

void loop_edogawa_master()
{
  display.clearDisplay();

  // read values from sensor
  float temperature = bme.readTemperature();
  char temp_s[6];
  dtostrf(temperature, 4, 1, temp_s);
  float humidity = bme.readHumidity();
  char hum_s[6];
  dtostrf(humidity, 4, 1, hum_s);
  uint16_t pressure = (uint16_t)(bme.readPressure() / 100.0F);

  Serial.print("Temp=");
  Serial.print(temp_s);
  Serial.print((char)0xE2);
  Serial.print((char)0x84);
  Serial.print((char)0x83); // DEGREE CELSIUS symbol in UTF-8
  Serial.print(" , Humidity=");
  Serial.print(hum_s);
  Serial.print("%, Pressure=");
  Serial.print(pressure);
  Serial.println("hPa");

  // Temperature
  char* decimal_part = strchr(temp_s, '.');

  if (decimal_part) {
    // decimal part first
    display.setTextSize(2);
    display.setCursor(58, 0);
    if (cnt % 2 == 1) *decimal_part = ' ';
    display.print(decimal_part);

    *decimal_part = '\0';
  }
  cnt++;

  display.setCursor(0, 0);
  display.setTextSize(5);
  display.print(temp_s);

  const uint8_t x = 58, y = 20;
  for (int i = 2; i <= 3; i++) {
    display.drawCircle(x + 4, y + 3, i, 1);
  }
  display.setCursor(x + 10, y);
  display.setTextSize(2);
  display.println("C");

  // Humidity
  strcat(hum_s, "%");
  display.setCursor(90,0);
  display.setTextSize(1);
  display.print(hum_s);

  // Pressure
  display.setCursor(90, 40);
  display.print(pressure);
  display.setCursor(90, 50);
  display.print("hPa");

  // 江戸川装置(OLED)
  for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
    display.drawRect(10 * i, 54, 9, 9, 1);
    if (edg[i].is_power_on()) display.fillRect(10 * i + 1, 55, 7, 7, 1);
  }

  display.display();

  if (tft) {
    // TFT
    thermometer_print_values(temperature, humidity, pressure);
    // 江戸川装置(TFT)
    for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
      tft.fillRect(20 * i + 1, tft.height() - 18, 17, 17, edg[i].is_power_on()? TFT_RED : TFT_WHITE);
    }
  }

  unsigned long currentTime = millis();
  if (lastEdogawaUnitOperation + EDOGAWA_UNIT_OPERATION_INTERVAL_SECS * 1000L < currentTime) {
    if (temperature < LOW_TEMPERATURE) {
      for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
        if (edg[i].is_power_on()) continue;
        // else
        Serial.print("Power on through edogawa unit #");
        Serial.println(i);
        if (edg[i].power_on()) {
          lastEdogawaUnitOperation = currentTime;
          break;
        }
      }
      lastEdogawaUnitOperation = millis();
    } else if (temperature > HIGH_TEMPERATURE) {
      for (int i = NUM_EDOGAWA_UNIT - 1; i >= 0; i--) {
        if (edg[i].is_power_on()) {
          Serial.print("Power off through edogawa unit #");
          Serial.println(i);
          if (edg[i].power_off()) lastEdogawaUnitOperation = currentTime;
          break;
        }
      }
    }
  }

  delay(1000);
}
