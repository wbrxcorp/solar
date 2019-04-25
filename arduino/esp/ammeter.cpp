#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include "globals.h"

static bool ina219_started = false;

static bool start_ina219()
{
  Wire.beginTransmission(INA219_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println("INA219 at address 0x40 not found!");
    display.turnOff();
    return false;
  }
  // else
  ina219.begin();
  Serial.println("INA219 started.");
  display.turnOn();
  return true;
}

void setup_ammeter()
{
  WiFi.mode(WIFI_OFF);
#ifdef ARDUINO_ARCH_ESP32
  btStop();
#endif

  ina219_started = start_ina219();
}

void loop_ammeter()
{
  if (!ina219_started) {
    ina219_started = start_ina219();
  }

  if (ina219_started) {
    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();
    float loadvoltage = busvoltage + (shuntvoltage / 1000);

    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
    Serial.println("");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);

    char loadvoltage_str[10];
    dtostrf((double)loadvoltage, 7, 2, loadvoltage_str);
    strcat(loadvoltage_str, " V");
    display.println(loadvoltage_str);

    char current_str[10];
    if (current_mA > 1000.0) {
      dtostrf((double)current_mA / 1000.0, 7, 2, current_str);
      strcat(current_str, " A");
    } else {
      dtostrf((double)current_mA, 7, 2, current_str);
      strcat(current_str, "mA");
    }
    display.println(current_str);

    char power_str[10];
    if (power_mW > 1000.0) {
      dtostrf((double)power_mW / 1000.0, 7, 2, power_str);
      strcat(power_str, " W");
    } else {
      dtostrf((double)power_mW, 7, 2, power_str);
      strcat(power_str, "mW");
    }
    display.println(power_str);
    display.display();
  }

  //delay(1000);
}
