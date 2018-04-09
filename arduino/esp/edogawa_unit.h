#ifndef __EDOGAWA_UNIT_H__
#define __EDOGAWA_UNIT_H__

#include <Arduino.h>

class EdogawaUnit {
  int swPin, ledPin;
  uint16_t acpiShutdownTimeout, forceShutdownTimeout;
public:
  EdogawaUnit(int _swPin, int _ledPin, uint16_t _acpiShutdownTimeout = 10000, uint16_t _forceShutdownTimeout = 6000) : swPin(_swPin), ledPin(_ledPin), acpiShutdownTimeout(_acpiShutdownTimeout), forceShutdownTimeout(_forceShutdownTimeout) { ; }
  bool is_power_on() { return digitalRead(ledPin) == LOW; }
  void power_on()
  {
    if (is_power_on()) return;
    digitalWrite(swPin, LOW);
    epsolar.load_on(true); // main power on
    delay(500);
    digitalWrite(swPin, HIGH);
    delay(200);
    digitalWrite(swPin, LOW);
  }

  void power_off()
  {
    if (!is_power_on()) return;
    // else
    digitalWrite(swPin, LOW);
    delay(100);
    digitalWrite(swPin, HIGH);
    delay(200);
    digitalWrite(swPin, LOW);
    // Wait for ACPI shutdown
    unsigned long time = millis();
    while (is_power_on()) {
      if (millis() - time > acpiShutdownTimeout) {
        // force OFF
        time = millis();
        digitalWrite(swPin, HIGH);
        while (is_power_on()) {
          if (millis() - time > forceShutdownTimeout) break;
          // else
          delay(100);
        }
        digitalWrite(swPin, LOW);
        break;
      }
      delay(100);
    }
  }
};

#endif // __EDOGAWA_UNIT_H__
