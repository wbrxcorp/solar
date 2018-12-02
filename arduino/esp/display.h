#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Adafruit_GFX.h>

const uint8_t DISPLAY_WIDTH = 128;
const uint8_t DISPLAY_HEIGHT = 64;

class Display : public Adafruit_GFX {
  int8_t i2caddr;
public:
  Display() : Adafruit_GFX(DISPLAY_WIDTH, DISPLAY_HEIGHT) { ; }

  void begin(uint8_t _i2caddr);

  void clearDisplay();
  void invertDisplay(bool invert);
  void display();

  void drawPixel(int16_t x, int16_t y, uint16_t color);

  void turnOn();
  void turnOff();
};

#endif /* __DISPLAY_H__ */
