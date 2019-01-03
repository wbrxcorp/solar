//#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

static Adafruit_INA219 ina219(INA219_ADDRESS);
static bool ina219_started = false;

static bool start_ina219()
{
  Wire.beginTransmission(INA219_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println("INA219 at address 0x40 not found!");
    return false;
  }
  // else
  ina219.begin();
  Serial.println("INA219 started.");
  return true;
}

void setup_ammeter()
{
  Wire.begin();
  ina219_started = start_ina219();
}

void loop_ammeter()
{
  if (!ina219_started) {
    ina219_started = start_ina219();
  }

  if (ina219_statred) {
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
  }

  delay(2000);
}
