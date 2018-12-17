#include <FS.h>
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

const int8_t logo240x240rle[] PROGMEM = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 9, 15, 40,
  55, 9, 0, 6, 58, 57, 7, 0, 8, 47, 1, 8, 38, 15, 11, 64, 42, 14, 8, 38,
  2, 24, 22, 15, 27, 64, 26, 15, 23, 22, 3, 39, 6, 15, 43, 64, 10, 15, 39, 5,
  4, 44, 11, 0, 4, 60, 59, 5, 0, 9, 44, 5, 6, 37, 15, 12, 64, 43, 15, 6,
  37, 5, 22, 21, 15, 28, 64, 27, 15, 22, 20, 6, 38, 5, 14, 45, 64, 12, 14, 38,
  4, 7, 41, 12, 0, 3, 61, 60, 4, 0, 11, 40, 8, 5, 36, 15, 13, 64, 44, 15,
  5, 35, 9, 20, 20, 14, 30, 64, 29, 14, 21, 19, 9, 36, 3, 15, 46, 64, 13, 15,
  36, 2, 10, 39, 13, 0, 2, 62, 61, 3, 0, 12, 38, 11, 3, 35, 14, 15, 64, 46,
  14, 4, 33, 12, 19, 18, 15, 31, 64, 30, 15, 19, 17, 13, 34, 2, 15, 47, 64, 14,
  15, 35, 1, 13, 36, 14, 64, 63, 1, 0, 13, 35, 14, 2, 33, 15, 16, 64, 47, 15,
  2, 32, 15, 17, 17, 15, 32, 64, 31, 15, 18, 15, 16, 33, 1, 14, 49, 64, 16, 14,
  33, 1, 0, 16, 32, 15, 1, 64, 64, 0, 15, 32, 17, 32, 15, 17, 64, 48, 15, 1,
  30, 18, 16, 16, 14, 34, 64, 33, 14, 17, 14, 19, 30, 1, 0, 14, 50, 64, 17, 15,
  29, 3, 0, 17, 30, 15, 2, 64, 64, 1, 15, 29, 19, 0, 2, 29, 14, 19, 64, 50,
  14, 28, 22, 14, 14, 15, 35, 64, 34, 14, 16, 12, 23, 27, 2, 0, 13, 51, 64, 18,
  15, 27, 4, 0, 19, 27, 14, 4, 64, 64, 3, 14, 26, 21, 0, 3, 26, 15, 20, 64,
  51, 13, 0, 1, 26, 25, 12, 13, 15, 36, 64, 35, 15, 14, 10, 26, 25, 3, 0, 11,
  53, 64, 20, 14, 24, 6, 0, 21, 23, 15, 5, 64, 64, 4, 14, 24, 22, 0, 5, 23,
  15, 21, 64, 52, 12, 0, 3, 22, 28, 11, 12, 14, 38, 64, 37, 14, 13, 9, 29, 22,
  4, 0, 10, 54, 64, 21, 14, 21, 8, 0, 22, 21, 15, 6, 64, 64, 5, 15, 20, 24,
  0, 7, 20, 14, 23, 64, 54, 10, 0, 4, 20, 31, 9, 11, 14, 39, 64, 38, 14, 12,
  7, 16, 1, 15, 19, 6, 0, 9, 55, 64, 22, 15, 18, 9, 0, 7, 1, 16, 18, 14,
  8, 64, 64, 7, 14, 17, 16, 2, 8, 0, 8, 18, 14, 24, 64, 55, 9, 0, 5, 17,
  16, 3, 16, 7, 9, 15, 40, 64, 39, 15, 10, 6, 15, 4, 16, 16, 7, 0, 7, 57,
  64, 24, 14, 15, 11, 0, 5, 5, 15, 16, 14, 9, 64, 64, 8, 14, 15, 16, 5, 6,
  0, 10, 14, 15, 25, 64, 56, 8, 0, 7, 13, 16, 6, 16, 6, 8, 14, 42, 64, 41,
  14, 9, 4, 16, 7, 16, 13, 8, 0, 6, 58, 64, 25, 14, 12, 13, 0, 3, 8, 16,
  12, 15, 10, 64, 64, 9, 15, 11, 16, 9, 4, 0, 12, 11, 14, 27, 64, 58, 6, 0,
  8, 11, 16, 9, 16, 4, 7, 14, 43, 64, 42, 14, 8, 2, 16, 10, 16, 10, 10, 0,
  5, 59, 64, 26, 15, 9, 14, 0, 2, 11, 16, 9, 14, 12, 64, 64, 11, 14, 8, 17,
  11, 3, 0, 13, 9, 14, 28, 64, 59, 5, 0, 9, 8, 16, 12, 17, 2, 5, 15, 44,
  64, 43, 15, 6, 1, 16, 13, 16, 7, 11, 0, 3, 61, 64, 28, 14, 6, 16, 14, 16,
  7, 14, 13, 64, 64, 12, 14, 6, 16, 15, 1, 0, 15, 5, 15, 29, 64, 60, 4, 0,
  10, 5, 17, 15, 16, 1, 4, 14, 46, 64, 45, 14, 4, 1, 0, 15, 16, 17, 4, 12,
  0, 2, 62, 64, 29, 14, 4, 16, 1, 16, 16, 3, 15, 14, 64, 64, 13, 14, 3, 16,
  18, 0, 16, 3, 14, 31, 64, 62, 2, 0, 12, 2, 16, 19, 15, 0, 1, 2, 14, 47,
  64, 46, 14, 1, 3, 0, 14, 19, 16, 1, 14, 0, 1, 63, 64, 30, 14, 1, 16, 3,
  17, 31, 16, 64, 64, 15, 30, 19, 2, 30, 32, 64, 63, 1, 0, 29, 21, 14, 0, 16,
  48, 64, 47, 17, 0, 12, 22, 29, 1, 64, 64, 32, 28, 4, 19, 28, 17, 64, 64, 16,
  27, 21, 3, 28, 33, 64, 64, 0, 27, 25, 12, 0, 14, 50, 64, 49, 15, 0, 11, 25,
  26, 2, 64, 64, 33, 25, 6, 20, 26, 18, 64, 64, 17, 25, 22, 5, 24, 35, 64, 64,
  2, 23, 28, 11, 0, 13, 51, 64, 50, 14, 0, 9, 29, 23, 3, 64, 64, 34, 23, 7,
  22, 22, 20, 64, 64, 19, 21, 24, 6, 22, 36, 64, 64, 3, 21, 31, 9, 0, 12, 52,
  64, 51, 13, 0, 7, 32, 20, 5, 64, 64, 36, 19, 9, 23, 20, 21, 64, 64, 20, 19,
  25, 8, 19, 37, 64, 64, 4, 18, 34, 8, 0, 10, 54, 64, 53, 11, 0, 6, 35, 17,
  6, 64, 64, 37, 17, 10, 25, 17, 22, 64, 64, 21, 16, 27, 9, 16, 39, 64, 64, 6,
  15, 37, 6, 0, 9, 55, 64, 54, 10, 0, 4, 38, 15, 7, 64, 64, 38, 14, 12, 27,
  13, 24, 64, 64, 23, 13, 28, 11, 13, 40, 64, 64, 7, 12, 40, 5, 0, 8, 56, 64,
  55, 9, 0, 3, 41, 11, 9, 64, 64, 40, 10, 14, 28, 11, 25, 64, 64, 24, 10, 30,
  12, 11, 41, 64, 64, 8, 10, 43, 3, 0, 6, 58, 64, 57, 7, 0, 1, 44, 9, 10,
  64, 64, 41, 8, 15, 30, 8, 26, 64, 64, 25, 8, 31, 14, 7, 43, 64, 64, 10, 6,
  46, 2, 0, 5, 59, 64, 58, 6, 47, 6, 11, 64, 64, 42, 5, 17, 31, 5, 28, 64,
  64, 27, 4, 33, 16, 4, 44, 64, 64, 11, 4, 49, 0, 4, 60, 64, 59, 3, 2, 48,
  3, 13, 64, 64, 44, 2, 18, 33, 2, 29, 64, 64, 28, 1, 35, 17, 2, 45, 64, 64,
  12, 1, 51, 1, 1, 62, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 34, 3, 6, 3, 18, 14,
  3, 47, 64, 29, 4, 9, 4, 10, 3, 5, 18, 3, 6, 3, 32, 2, 0, 1, 63, 64,
  13, 4, 9, 4, 10, 3, 21, 2, 3, 6, 3, 32, 3, 15, 64, 62, 2, 0, 1, 9,
  5, 9, 3, 23, 3, 6, 3, 2, 30, 3, 31, 64, 46, 3, 9, 5, 1, 7, 4, 23,
  3, 6, 3, 18, 64, 64, 30, 3, 8, 3, 1, 2, 8, 3, 6, 18, 3, 6, 3, 34,
  64, 64, 14, 4, 7, 3, 1, 2, 8, 3, 22, 2, 3, 6, 3, 50, 64, 63, 1, 0,
  2, 7, 3, 1, 3, 7, 3, 7, 7, 10, 3, 6, 3, 2, 1, 5, 10, 3, 3, 5,
  3, 3, 5, 3, 9, 4, 10, 64, 47, 3, 7, 2, 2, 3, 6, 4, 4, 12, 8, 3,
  6, 3, 1, 9, 8, 0, 3, 2, 6, 3, 3, 6, 3, 8, 3, 27, 64, 31, 3, 6,
  3, 3, 2, 6, 3, 5, 2, 0, 11, 7, 3, 6, 3, 1, 10, 7, 3, 1, 7, 3,
  2, 0, 1, 6, 4, 6, 3, 44, 64, 15, 4, 5, 3, 3, 2, 6, 3, 5, 1, 9,
  3, 5, 2, 3, 6, 5, 6, 4, 6, 5, 9, 3, 7, 4, 4, 0, 4, 60, 64, 0,
  3, 5, 3, 3, 3, 5, 3, 16, 3, 6, 3, 6, 4, 1, 7, 3, 6, 4, 10, 3,
  8, 3, 3, 4, 13, 64, 48, 3, 5, 2, 4, 2, 0, 1, 5, 3, 16, 3, 6, 3,
  6, 4, 8, 4, 5, 0, 4, 10, 3, 8, 4, 2, 3, 30, 64, 32, 3, 4, 3, 4,
  3, 4, 4, 7, 9, 3, 6, 3, 6, 3, 10, 3, 5, 3, 11, 2, 0, 1, 9, 8,
  46, 64, 16, 4, 3, 3, 5, 2, 4, 3, 9, 11, 4, 2, 3, 6, 3, 10, 3, 5,
  3, 11, 3, 10, 5, 0, 1, 63, 64, 1, 3, 3, 3, 5, 3, 3, 3, 7, 13, 6,
  3, 6, 3, 2, 8, 3, 5, 3, 11, 3, 11, 4, 16, 64, 49, 3, 3, 2, 6, 1,
  0, 2, 3, 3, 6, 6, 5, 3, 6, 3, 6, 3, 10, 3, 5, 0, 3, 11, 3, 11,
  4, 32, 64, 33, 3, 3, 2, 6, 3, 2, 4, 6, 2, 0, 1, 8, 3, 6, 3, 6,
  3, 10, 3, 5, 3, 11, 2, 0, 1, 10, 5, 48, 64, 17, 4, 1, 3, 7, 2, 2,
  3, 6, 3, 9, 3, 4, 2, 3, 6, 3, 10, 3, 5, 3, 11, 3, 9, 6, 0, 1,
  63, 64, 2, 3, 1, 3, 7, 3, 1, 3, 6, 3, 9, 3, 6, 3, 6, 3, 2, 8,
  3, 5, 3, 11, 3, 9, 3, 1, 4, 14, 64, 50, 3, 1, 3, 7, 0, 3, 1, 3,
  6, 3, 9, 3, 6, 3, 6, 4, 8, 4, 5, 0, 3, 11, 3, 8, 3, 3, 4, 29,
  64, 34, 6, 8, 7, 6, 3, 8, 4, 6, 3, 6, 4, 8, 3, 6, 3, 11, 2, 0,
  1, 7, 4, 4, 3, 45, 64, 18, 6, 9, 5, 7, 4, 6, 5, 4, 2, 3, 6, 5,
  6, 4, 6, 3, 11, 3, 6, 4, 5, 0, 4, 60, 64, 3, 5, 9, 5, 8, 14, 6,
  3, 6, 3, 1, 1, 0, 9, 7, 3, 11, 3, 6, 3, 7, 4, 11, 64, 51, 5, 8,
  1, 5, 9, 9, 1, 3, 6, 3, 6, 3, 1, 9, 8, 0, 3, 11, 3, 5, 4, 8,
  3, 27, 64, 35, 4, 10, 5, 10, 0, 6, 3, 3, 6, 3, 6, 3, 3, 5, 10, 3,
  11, 2, 0, 1, 4, 4, 10, 3, 42, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  44, 3, 17, 64, 18, 7, 39, 64, 28, 3, 33, 63, 1, 0, 12, 52, 63, 1, 0, 2,
  10, 3, 49, 46, 15, 3, 64, 47, 3, 10, 3, 1, 64, 29, 5, 8, 3, 19, 64, 31,
  3, 30, 64, 12, 4, 12, 1, 35, 64, 15, 3, 46, 59, 4, 1, 64, 63, 1, 0, 2,
  62, 43, 4, 17, 6, 6, 10, 3, 3, 5, 3, 3, 3, 5, 14, 3, 0, 3, 10, 3,
  3, 5, 6, 7, 8, 11, 4, 3, 1, 9, 6, 10, 3, 3, 5, 28, 27, 3, 22, 10,
  2, 6, 3, 2, 6, 3, 3, 1, 9, 10, 10, 8, 3, 2, 6, 3, 12, 6, 11, 4,
  3, 8, 9, 0, 1, 8, 3, 2, 8, 42, 10, 4, 21, 12, 7, 3, 1, 6, 0, 1,
  3, 3, 1, 10, 8, 12, 7, 3, 1, 7, 3, 5, 0, 8, 7, 3, 10, 3, 7, 12,
  7, 3, 1, 3, 0, 7, 51, 4, 2, 18, 4, 6, 4, 6, 5, 9, 5, 6, 1, 0,
  3, 6, 4, 6, 4, 6, 5, 9, 1, 9, 3, 7, 1, 0, 2, 10, 3, 6, 4, 6,
  4, 6, 5, 6, 3, 9, 42, 3, 19, 2, 3, 8, 3, 6, 4, 10, 4, 8, 3, 6,
  3, 4, 4, 3, 6, 4, 21, 3, 6, 3, 10, 3, 1, 5, 3, 8, 3, 6, 4, 7,
  4, 24, 26, 3, 20, 4, 8, 3, 0, 1, 5, 4, 10, 4, 8, 4, 4, 4, 8, 4,
  5, 3, 0, 1, 21, 3, 6, 3, 10, 3, 5, 4, 8, 0, 4, 5, 4, 8, 3, 40,
  10, 3, 20, 3, 10, 3, 5, 3, 7, 4, 3, 10, 3, 4, 3, 10, 3, 5, 3, 16,
  6, 3, 6, 3, 10, 3, 5, 3, 10, 3, 5, 3, 4, 5, 3, 50, 3, 3, 17, 3,
  10, 3, 5, 3, 11, 3, 9, 1, 3, 4, 3, 10, 3, 5, 3, 14, 11, 6, 1, 0,
  2, 10, 3, 5, 3, 10, 3, 5, 3, 9, 3, 8, 42, 3, 19, 1, 3, 10, 3, 5,
  3, 11, 3, 10, 3, 4, 3, 5, 5, 3, 5, 3, 12, 13, 6, 3, 10, 3, 1, 4,
  3, 10, 3, 5, 3, 9, 3, 24, 26, 4, 19, 3, 10, 2, 0, 1, 5, 3, 11, 3,
  10, 3, 4, 3, 10, 3, 5, 3, 11, 6, 5, 3, 6, 3, 10, 3, 5, 3, 9, 1,
  3, 5, 3, 9, 3, 40, 10, 4, 19, 3, 10, 3, 5, 3, 7, 4, 3, 10, 3, 4,
  3, 10, 3, 5, 3, 11, 3, 2, 6, 3, 6, 3, 10, 3, 5, 3, 10, 3, 5, 3,
  4, 5, 3, 51, 3, 2, 17, 3, 10, 3, 5, 3, 11, 3, 9, 1, 3, 4, 3, 10,
  3, 5, 3, 10, 3, 9, 3, 6, 1, 0, 2, 10, 3, 5, 3, 10, 3, 5, 3, 9,
  3, 8, 43, 4, 17, 1, 3, 10, 3, 5, 3, 11, 3, 10, 3, 4, 3, 5, 5, 3,
  5, 3, 10, 3, 9, 3, 6, 3, 10, 3, 1, 4, 3, 10, 3, 5, 3, 9, 3, 24,
  27, 4, 18, 4, 8, 3, 0, 1, 5, 3, 11, 4, 8, 4, 4, 4, 8, 4, 5, 3,
  10, 3, 9, 3, 6, 3, 10, 3, 5, 4, 8, 0, 4, 5, 3, 9, 3, 40, 12, 4,
  12, 1, 5, 3, 8, 3, 6, 3, 7, 4, 4, 8, 3, 6, 3, 8, 3, 6, 3, 10,
  3, 3, 5, 4, 6, 3, 10, 3, 6, 3, 8, 3, 6, 3, 4, 5, 3, 53, 3, 0,
  2, 8, 3, 5, 4, 6, 4, 6, 3, 11, 5, 6, 1, 0, 3, 6, 4, 6, 4, 6,
  3, 10, 4, 6, 5, 6, 1, 0, 3, 9, 3, 6, 4, 6, 4, 6, 3, 9, 3, 8,
  46, 15, 3, 3, 12, 7, 3, 11, 3, 1, 10, 8, 6, 0, 6, 7, 3, 11, 14, 7,
  8, 4, 3, 1, 6, 12, 7, 3, 9, 3, 24, 31, 13, 8, 10, 2, 6, 3, 11, 3,
  1, 9, 10, 10, 8, 3, 12, 9, 1, 3, 7, 8, 4, 3, 8, 9, 0, 1, 8, 3,
  9, 3, 40, 18, 7, 13, 6, 10, 3, 7, 4, 3, 3, 5, 14, 6, 10, 3, 13, 3,
  0, 3, 3, 3, 9, 6, 4, 3, 10, 6, 10, 3, 4, 5, 3, 56, 52, 3, 9, 64,
  64, 64, 36, 3, 25, 64, 64, 64, 20, 3, 41, 64, 64, 64, 4, 3, 57, 64, 64, 52,
  3, 9, 64, 64, 64, 36, 3, 25, 64, 64, 64, 20, 3, 41, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64
};

/*
#!/usr/bin/python2
import sys,itertools
import cv2,numpy
chunks = numpy.array_split(list(itertools.chain.from_iterable(cv2.imread("logo240.pbm", cv2.IMREAD_UNCHANGED))), 240 * 240 / 64)

compressed = []
for chunk in chunks:
  current_pixel = 255 # white
  cnt = 0
  for pixel in chunk:
    if current_pixel == pixel:
      cnt += 1
      continue
    # else
    compressed.append(cnt)
    cnt = 1
    current_pixel = 0 if current_pixel == 255 else 255
  compressed.append(cnt)

cnt = 0
for num in compressed:
  sys.stdout.write("%d, " % num)
  cnt += 1
  if cnt % 20 == 0: print
*/

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
  _spi->writePixels((uint8_t*)colors , len * 2);
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


void TFT::begin(SPIClass *spiClass, int8_t dc, int8_t rst/* = -1*/, int8_t cs/* = -1*/, uint32_t freq/* = TFT_SPI_DEFAULT_FREQ*/) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _spi = spiClass;

  if(!freq) {
    freq = TFT_SPI_DEFAULT_FREQ;
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

  started = true;
  setRotation(initialRotation);

  // show splash
  startWrite();

  uint16_t  buffer[64];
  const uint8_t bufferSize = sizeof(buffer) / sizeof(buffer[0]);
  const int8_t* pt = logo240x240rle;
  uint8_t idx = 0;
  uint16_t currentPixel = TFT_WHITE;

  setAddrWindow(0, 0, TFT_WIDTH, TFT_HEIGHT);

  while (pt - logo240x240rle < sizeof(logo240x240rle)) {
    for (uint8_t i = 0; i < *pt && idx < bufferSize; i++) {
      buffer[idx] = currentPixel;
      idx++;
    }
    currentPixel = (currentPixel == TFT_WHITE)? TFT_BLACK : TFT_WHITE;
    if (idx >= bufferSize) {
      pushColors(buffer, bufferSize);
      idx = 0;
      currentPixel = TFT_WHITE;
    }
    pt++;
  }

  endWrite();
}

void TFT::begin(int8_t dc, int8_t rst/* = -1*/, int8_t cs/* = -1*/, uint32_t freq/* = TFT_SPI_DEFAULT_FREQ*/)
{
  begin(&SPI, dc, rst, cs, freq);
}


void TFT::setRotation(uint8_t rotation)
{
  rotation %= 4;

  if (!started) {
    initialRotation = rotation;
    return;
  }

  // else
  const struct {
    uint8_t madctl;
    int16_t xstart, ystart;
  } rotation_params[] = {
    { ST77XX_MADCTL_RGB | ST77XX_MADCTL_MX | ST77XX_MADCTL_MY, _colstart, _rowstart },
    { ST77XX_MADCTL_RGB | ST77XX_MADCTL_MY | ST77XX_MADCTL_MV, _rowstart, _colstart },
    { ST77XX_MADCTL_RGB, 0, 0 },
    { ST77XX_MADCTL_RGB | ST77XX_MADCTL_MX | ST77XX_MADCTL_MV, 0, 0 }
  };

  _xstart = rotation_params[rotation].xstart;
  _ystart = rotation_params[rotation].ystart;

  startWrite();
  writeCommand(ST77XX_MADCTL);
  _spi->write(rotation_params[rotation].madctl);
  endWrite();
}

bool TFT::showBitmapFile(Stream& f)
{
  struct __attribute__((__packed__)) {
    char bm[2];
    uint32_t filesize;
    uint16_t reserved[2];
    uint32_t offset;

    uint32_t hdrsize; // >= 40
    uint32_t width; // == 240
    int32_t height; // == 240 | -240
    uint16_t planes; // == 1
    uint16_t depth;  // == 16
    uint32_t compression; // == 3

    uint32_t datasize,hresoltion,vresolution,usedcolors,importabtcolors;
    uint32_t colormask_r,colormask_g,colormask_b;
  } bmpheader;

  if (f.readBytes((char*)&bmpheader, sizeof(bmpheader)) != sizeof(bmpheader)) {
    Serial.println("Bitmap file too short");
    return false;
  }
  // else
  if (bmpheader.bm[0] != 'B' || bmpheader.bm[1] != 'M' || bmpheader.hdrsize < 108) {
    Serial.println("Incorrect bitmap file header");
    Serial.print("identifier: "); Serial.print(bmpheader.bm[0]); Serial.println(bmpheader.bm[1]);
    Serial.print("hdrsize: "); Serial.println(bmpheader.hdrsize);
    return false;
  }
  // else
  if (bmpheader.planes != 1 || bmpheader.width == 0 || bmpheader.height == 0
    || bmpheader.width > _width || bmpheader.height > _height || -bmpheader.height > _height
    || bmpheader.depth != 16) {
    Serial.println("Invalid image size or format");
    return false;
  }
  // else
  if (bmpheader.compression != 3) {
    Serial.println("Unsupprted compression type(must be 3:BI_BITFIELDS");
    return false;
  }

  if (bmpheader.colormask_r != 0x0000f800 || bmpheader.colormask_g != 0x000007E0 || bmpheader.colormask_b != 0x0000001f) {
    Serial.println("Invalid pixel format(must be RGB565)");
    return false;
  }

  // skip
  for (int i = sizeof(bmpheader); i < bmpheader.offset; i++) {
    if (f.read() < 0) {
      Serial.print("File seek error(probably file is too short). offset="); Serial.println(bmpheader.offset);
      return false;
    };
  }

  startWrite();
  for (int i = 0; i < (bmpheader.height > 0? bmpheader.height : -bmpheader.height); i++) {
    uint16_t line[bmpheader.width];
    uint16_t y = bmpheader.height > 0? (bmpheader.height - i - 1) : i;
    if (f.readBytes((char*)line, sizeof(line)) < sizeof(line)) {
      Serial.print("Bitmap size is too short. y="); Serial.println(y);
      return false;
    }

    setAddrWindow(0, y, bmpheader.width, 1);
    uint16_t* pt;
    for (pt = line; pt - line + 64 < bmpheader.width; pt += 64) {
      pushColors(pt, 64);
    }
    pushColors(pt, bmpheader.width % 64);
  }
  endWrite();

  return true;
}
