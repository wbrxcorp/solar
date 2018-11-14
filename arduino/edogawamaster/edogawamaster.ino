// arduino --upload --board arduino:avr:uno --port /dev/ttyACM0 edogawamaster.ino
// arduino --upload --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyACM0 edogawamaster.ino

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "display.h"
#include "edogawa_unit.h"

#define DISPLAY_I2C_ADDRESS 0x3c
#define BME280_I2C_ADDRESS 0x76 // GYBMEP

Display display;
Adafruit_BME280 bme; // I2C

const uint8_t NUM_EDOGAWA_UNIT = 5;
EdogawaUnit edg[NUM_EDOGAWA_UNIT];
uint8_t swPin[NUM_EDOGAWA_UNIT] = { 2, 4, 6, 8, 10 };
uint8_t ledPin[NUM_EDOGAWA_UNIT] = { 3, 5, 7, 9, 11 };
unsigned long lastEdogawaUnitOperation = 0;
const uint16_t EDOGAWA_UNIT_OPERATION_INTERVAL_SECS = 300;

const float LOW_TEMPERATURE = 22.5;
const float HIGH_TEMPERATURE = 25.5;

void setup()
{
  Serial.begin(115200);

  display.begin(DISPLAY_I2C_ADDRESS);
  display.display();

  if (!bme.begin(BME280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    display.print("SENSOR NOT FOUND");
    display.display();
    while (1);
  }
  // else
  Serial.println("Temperature sensor Initialized");

  for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
    edg[i].begin(swPin[i], ledPin[i]);
  }

  lastEdogawaUnitOperation = millis();
}

void loop()
{
  char buf[6];
  display.clearDisplay();

  // Temperature
  float temperature = bme.readTemperature();
  dtostrf(temperature, 4, 1, buf);
  char* decimal_part = strchr(buf, '.');

  if (decimal_part) {
    // decimal part first
    display.setTextSize(2);
    display.setCursor(58, 0);
    display.print(decimal_part);

    *decimal_part = '\0';
  }

  display.setCursor(0, 0);
  display.setTextSize(5);
  display.print(buf);

  const uint8_t x = 58, y = 20;
  for (int i = 2; i <= 3; i++) {
    display.drawCircle(x + 4, y + 3, i, 1);
  }
  display.setCursor(x + 10, y);
  display.setTextSize(2);
  display.println("C");

  // Humidity
  float humidity = bme.readHumidity();
  dtostrf(humidity, 4, 1, buf);
  strcat(buf, "%");
  display.setCursor(90,0);
  display.setTextSize(1);
  display.print(buf);

  // Pressure
  int pressure = (int)(bme.readPressure() / 100.0F);
  display.setCursor(90, 40);
  display.print(pressure);
  display.setCursor(90, 50);
  display.print("hPa");

  // 江戸川装置
  for (int i = 0; i < NUM_EDOGAWA_UNIT; i++) {
    display.drawRect(10 * i, 54, 9, 9, 1);
    if (edg[i].is_power_on()) display.fillRect(10 * i + 1, 55, 7, 7, 1);
  }

  display.display();

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
