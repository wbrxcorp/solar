#include "TFT.h"

#define ST_CMD_DELAY      0x80    // special signifier for command lists
#define ST77XX_SWRESET    0x01
#define ST77XX_SLPOUT     0x11
#define ST77XX_NORON      0x13
#define ST77XX_INVOFF     0x20
#define ST77XX_INVON      0x21
#define ST77XX_DISPON     0x29
#define ST77XX_CASET      0x2A
#define ST77XX_RASET      0x2B
#define ST77XX_RAMWR      0x2C
#define ST77XX_COLMOD     0x3A
#define ST77XX_MADCTL     0x36
#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_RGB 0x00

#ifdef ARDUINO_ARCH_ESP32
  #define SPI_HAS_WRITE_PIXELS
  #define SPI_MAX_PIXELS_AT_ONCE  32
#endif

const uint16_t ST7789_240x240_XSTART = 0;
const uint16_t ST7789_240x240_YSTART = 80;

uint16_t TFT::color565(uint8_t red, uint8_t green, uint8_t blue) {
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
}

void inline TFT::startWrite(void){
    _spi->beginTransaction(SPISettings(_freq, MSBFIRST, _spiDataMode));
    if(_cs >= 0) digitalWrite(_cs, HIGH);
}

void inline TFT::endWrite(void){
    if(_cs >= 0) digitalWrite(_cs, HIGH);
    _spi->endTransaction();
}

void TFT::writeCommand(uint8_t cmd){
    digitalWrite(_dc, LOW);
    _spi->write(cmd);
    digitalWrite(_dc, HIGH);
}

void TFT::writePixels(uint16_t *colors, uint32_t len) {
#ifdef SPI_HAS_WRITE_PIXELS
  _spi->writePixels((uint8_t*)colors , len * 2)
#else
  for(uint32_t i = 0; i < len; i++) {
    _spi->write16(((uint16_t*)colors)[i]);
  }
#endif
}

void TFT::writeColor(uint16_t color, uint32_t len) {

    if(!len) return; // Avoid 0-byte transfers

    uint8_t hi = color >> 8, lo = color;

  #ifdef SPI_HAS_WRITE_PIXELS
    #define TMPBUF_LONGWORDS (SPI_MAX_PIXELS_AT_ONCE + 1) / 2
    #define TMPBUF_PIXELS    (TMPBUF_LONGWORDS * 2)
    static uint32_t temp[TMPBUF_LONGWORDS];
    uint32_t        c32    = color * 0x00010001;
    uint16_t        bufLen = (len < TMPBUF_PIXELS) ? len : TMPBUF_PIXELS,
                    xferLen, fillLen;

    // Fill temp buffer 32 bits at a time
    fillLen = (bufLen + 1) / 2; // Round up to next 32-bit boundary
    for(uint32_t t=0; t<fillLen; t++) {
        temp[t] = c32;
    }

    // Issue pixels in blocks from temp buffer
    while(len) {                                 // While pixels remain
        xferLen = (bufLen < len) ? bufLen : len; // How many this pass?
        writePixels((uint16_t *)temp, xferLen);
        len -= xferLen;
    }
  #else
    while(len--) {
        _spi->write(hi);
        _spi->write(lo);
    }
  #endif
}

void TFT::writePixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    setAddrWindow(x,y,1,1);
    writePixel(color);
}

void TFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    if((x >= _width) || (y >= _height)) return;
    int16_t x2 = x + w - 1, y2 = y + h - 1;
    if((x2 < 0) || (y2 < 0)) return;

    // Clip left/top
    if(x < 0) {
        x = 0;
        w = x2 + 1;
    }
    if(y < 0) {
        y = 0;
        h = y2 + 1;
    }

    // Clip right/bottom
    if(x2 >= _width)  w = _width  - x;
    if(y2 >= _height) h = _height - y;

    setAddrWindow(x, y, w, h);
    writeColor(color, (int32_t)w * h);
}

void TFT::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  x += _xstart;
  y += _ystart;
  uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
  uint32_t ya = ((uint32_t)y << 16) | (y+h-1);

  writeCommand(ST77XX_CASET); // Column addr set
  _spi->write32(xa);

  writeCommand(ST77XX_RASET); // Row addr set
  _spi->write32(ya);

  writeCommand(ST77XX_RAMWR); // write to RAM
}

inline void TFT::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    writeFillRect(x, y, 1, h, color);
}

inline void TFT::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    writeFillRect(x, y, w, 1, color);
}

void TFT::drawPixel(int16_t x, int16_t y, uint16_t color){
    // Clip first...
    if((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
        // THEN set up transaction (if needed) and draw...
        startWrite();
        setAddrWindow(x, y, 1, 1);
        writePixel(color);
        endWrite();
    }
}

void TFT::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}

void TFT::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}

void TFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    startWrite();
    writeFillRect(x,y,w,h,color);
    endWrite();
}

void TFT::invertDisplay(boolean i) {
  startWrite();
  writeCommand(i ? ST77XX_INVON : ST77XX_INVOFF);
  endWrite();
}

void TFT::drawRGBBitmap(int16_t x, int16_t y,
  uint16_t *pcolors, int16_t w, int16_t h) {

    int16_t x2, y2; // Lower-right coord
    if(( x             >= _width ) ||      // Off-edge right
       ( y             >= _height) ||      // " top
       ((x2 = (x+w-1)) <  0      ) ||      // " left
       ((y2 = (y+h-1)) <  0)     ) return; // " bottom

    int16_t bx1=0, by1=0, // Clipped top-left within bitmap
            saveW=w;      // Save original bitmap width value
    if(x < 0) { // Clip left
        w  +=  x;
        bx1 = -x;
        x   =  0;
    }
    if(y < 0) { // Clip top
        h  +=  y;
        by1 = -y;
        y   =  0;
    }
    if(x2 >= _width ) w = _width  - x; // Clip right
    if(y2 >= _height) h = _height - y; // Clip bottom

    pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
    startWrite();
    setAddrWindow(x, y, w, h); // Clipped area
    while(h--) { // For each (clipped) scanline...
      writePixels(pcolors, w); // Push one (clipped) row
      pcolors += saveW; // Advance pointer by one full (unclipped) line
    }
    endWrite();
}

void TFT::pushColors(uint16_t *data, uint32_t len)
{
  startWrite();

#ifdef SPI_HAS_WRITE_PIXELS
  _spi->writePixels(data,len<<1);
#else

  uint32_t color[8];

  uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));

  SPI1U1 = (SPI1U1 & mask) | (255 << SPILMOSI) | (255 << SPILMISO);

  while(len>15)
  {
    uint32_t i = 0;
    while(i<8)
    {
      color[i]  = (*data >> 8) | (uint16_t)(*data << 8);
      data++;
      color[i] |= ((*data >> 8) | (*data << 8)) << 16;
      data++;
      i++;
    }

    len -= 16;

    // ESP8266 wait time here at 40MHz SPI is ~5.45us
    while(SPI1CMD & SPIBUSY) {}
    SPI1W0 = color[0];
    SPI1W1 = color[1];
    SPI1W2 = color[2];
    SPI1W3 = color[3];
    SPI1W4 = color[4];
    SPI1W5 = color[5];
    SPI1W6 = color[6];
    SPI1W7 = color[7];
    SPI1CMD |= SPIBUSY;
  }

  if(len)
  {
    uint32_t bits = (len*16-1); // bits left to shift - 1
    uint16_t* ptr = (uint16_t*)color;
    while(len--)
    {
      *ptr++ = (*(data) >> 8) | (uint16_t)(*(data) << 8);
      data++;
    }
    while(SPI1CMD & SPIBUSY) {}
    SPI1U1 = (SPI1U1 & mask) | (bits << SPILMOSI) | (bits << SPILMISO);
    SPI1W0 = color[0];
    SPI1W1 = color[1];
    SPI1W2 = color[2];
    SPI1W3 = color[3];
    SPI1W4 = color[4];
    SPI1W5 = color[5];
    SPI1W6 = color[6];
    SPI1W7 = color[7];
    SPI1CMD |= SPIBUSY;
  }

  while(SPI1CMD & SPIBUSY) {}

#endif

  endWrite();
}

void TFT::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, const uint16_t *data)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height)) return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0) { dw += x; dx = -x; x = 0; }
  if (y < 0) { dh += y; dy = -y; y = 0; }

  if ((x + w) > _width ) dw = _width  - x;
  if ((y + h) > _height) dh = _height - y;

  if (dw < 1 || dh < 1) return;

  startWrite();

  data += dx + dy * w;

  uint16_t  buffer[64];
  uint16_t* pix_buffer = buffer;

  setAddrWindow(x, y, dw, dh);

  // Work out the number whole buffers to send
  uint16_t nb = (dw * dh) / 64;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++) {
    for (int j = 0; j < 64; j++) {
      pix_buffer[j] = pgm_read_word(&data[i * 64 + j]);
    }
    pushColors(pix_buffer, 64);
  }

  // Work out number of pixels not yet sent
  uint16_t np = (dw * dh) % 64;

  // Send any partial buffer left over
  if (np) {
    for (int i = 0; i < np; i++)
    {
      pix_buffer[i] = pgm_read_word(&data[nb * 64 + i]);
    }
    pushColors(pix_buffer, np);
  }

  endWrite();
}

static const uint8_t PROGMEM
  cmd_240x240[] =  {                // Init commands for 7789 screens
    9,                              //  9 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      150,                          //    150 ms delay
    ST77XX_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      255,                          //     255 = 500 ms delay
    ST77XX_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST77XX_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
      0x00,
      ST7789_240x240_XSTART,        //     XSTART = 0
      (240+ST7789_240x240_XSTART)>>8,
      (240+ST7789_240x240_XSTART)&0xFF,  //     XEND = 240
    ST77XX_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
      0x00,
      ST7789_240x240_YSTART,             //     YSTART = 0
      (240+ST7789_240x240_YSTART)>>8,
      (240+ST7789_240x240_YSTART)&0xFF,  //     YEND = 240
    ST77XX_INVON  ,   ST_CMD_DELAY,  //  7: hack
      10,
    ST77XX_NORON  ,   ST_CMD_DELAY, //  8: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON ,   ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
    255 };                          //     255 = max (500 ms) delay


void TFT::begin(SPIClass *spiClass, int8_t dc, int8_t rst/* = -1*/, int8_t cs/* = -1*/, uint32_t freq/* = SPI_DEFAULT_FREQ*/) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _spi = spiClass;

  if(!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  _freq = freq;

  // Control Pins
  if(_cs >= 0) {
      pinMode(_cs, OUTPUT);
      digitalWrite(_cs, HIGH); // Deselect
  }
  pinMode(_dc, OUTPUT);
  digitalWrite(_dc, LOW);

  _spi->begin();

  // toggle RST low to reset
  if (_rst >= 0) {
      pinMode(_rst, OUTPUT);
      digitalWrite(_rst, HIGH);
      delay(5);
      digitalWrite(_rst, LOW);
      delay(20);
      digitalWrite(_rst, HIGH);
  }

  _colstart = ST7789_240x240_XSTART;
  _rowstart = ST7789_240x240_YSTART;
  _height   = TFT_WIDTH;
  _width    = TFT_HEIGHT;

  const uint8_t* addr = cmd_240x240;
  startWrite();
  uint8_t numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...

    writeCommand(pgm_read_byte(addr++)); // Read, issue command
    uint8_t numArgs  = pgm_read_byte(addr++);    // Number of args to follow
    uint16_t ms       = numArgs & ST_CMD_DELAY;   // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            // Mask out delay bit
    while(numArgs--) {                   // For each argument...
      _spi->write(pgm_read_byte(addr++));   // Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
  endWrite();

  setRotation(0);
}

void TFT::begin(int8_t dc, int8_t rst/* = -1*/, int8_t cs/* = -1*/, uint32_t freq/* = SPI_DEFAULT_FREQ*/)
{
  begin(&SPI, dc, rst, cs, freq);
}


void TFT::setRotation(uint8_t m)
{
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation) {
   case 0:
     madctl  = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
     _xstart = _colstart;
     _ystart = _rowstart;
     break;
   case 1:
     madctl  = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
     _xstart = _rowstart;
     _ystart = _colstart;
     break;
  case 2:
     madctl  = ST77XX_MADCTL_RGB;
     _xstart = 0;
     _ystart = 0;
     break;
   case 3:
     madctl  = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
     _xstart = 0;
     _ystart = 0;
     break;
  }
  startWrite();
  writeCommand(ST77XX_MADCTL);
  _spi->write(madctl);
  endWrite();
}
