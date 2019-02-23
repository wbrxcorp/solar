#ifndef __TFT_H__
#define __TFT_H__

#include <SPI.h>
#include <Adafruit_GFX.h>

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define	TFT_BLACK      0x0000
#define TFT_WHITE      0xFFFF
#define	TFT_RED        0xF800
#define	TFT_GREEN      0x07E0
#define	TFT_BLUE       0x001F
#define TFT_CYAN       0x07FF
#define TFT_MAGENTA    0xF81F
#define TFT_YELLOW     0xFFE0
#define	TFT_ORANGE     0xFC00

// Default SPI data clock frequency
#if (F_CPU >= 80000000)
  #define TFT_SPI_DEFAULT_FREQ 80000000
#else
  #define TFT_SPI_DEFAULT_FREQ F_CPU * 2 / 3
#endif

class TFT : public Adafruit_GFX {
  SPIClass *_spi;
  uint32_t _freq;
  uint8_t _spiDataMode = SPI_MODE3;
  int32_t  _cs, _dc, _rst;
	int16_t  _xstart = 0, _ystart = 0;
  uint8_t _colstart = 0, _rowstart = 0;
  bool started = false;
  uint8_t initialRotation = 2;

protected:
  void writeCommand(uint8_t cmd);
  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

public:
  TFT() : Adafruit_GFX(TFT_WIDTH, TFT_HEIGHT) { ; }
  void setDataMode(uint8_t spiDataMode) { _spiDataMode = spiDataMode; }
  void begin(SPIClass *spiClass, int8_t dc, int8_t rst = -1, int8_t cs = -1, uint32_t freq = TFT_SPI_DEFAULT_FREQ);
  void begin(int8_t dc, int8_t rst = -1, int8_t cs = -1, uint32_t freq = TFT_SPI_DEFAULT_FREQ);

  operator bool() const { return started; }

  void setRotation(uint8_t m);

  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void startWrite(void);
  void endWrite(void);

  void writePixel(int16_t x, int16_t y, uint16_t color);
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

	inline void writePixel(uint16_t color) { _spi->write16(color); }
  void writePixels(uint16_t * colors, uint32_t len);
  void writeColor(uint16_t color, uint32_t len);

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  using Adafruit_GFX::drawRGBBitmap; // Check base class first
  void drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h);

  void pushColors(uint16_t *data, uint32_t len);
  void pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, const uint16_t *data);
  bool showBitmapFile(Stream& f);
	void invertDisplay(boolean i);

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
};

#endif // __TFT_H__
