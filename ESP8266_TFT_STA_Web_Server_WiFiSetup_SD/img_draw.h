#pragma once
#include <Arduino.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Должен быть объявлен в твоём .ino
extern Adafruit_ST7735 tft;

static uint16_t _rd16(File &f) {
  uint16_t r;
  ((uint8_t*)&r)[0] = f.read();
  ((uint8_t*)&r)[1] = f.read();
  return r;
}
static uint32_t _rd32(File &f) {
  uint32_t r;
  ((uint8_t*)&r)[0] = f.read();
  ((uint8_t*)&r)[1] = f.read();
  ((uint8_t*)&r)[2] = f.read();
  ((uint8_t*)&r)[3] = f.read();
  return r;
}

// Рисует BMP по координатам (x,y). Поддержка 24-bit и 16-bit (RGB565), без сжатия.
static bool drawBmpFromSD(const char* filename, int16_t x, int16_t y) {
  File bmp = SD.open(filename, FILE_READ);
  if (!bmp) return false;

  if (_rd16(bmp) != 0x4D42) { bmp.close(); return false; } // 'BM'
  (void)_rd32(bmp); // fileSize
  (void)_rd32(bmp); // reserved
  uint32_t dataOff = _rd32(bmp);

  (void)_rd32(bmp); // headerSize
  int32_t w = (int32_t)_rd32(bmp);
  int32_t h = (int32_t)_rd32(bmp);

  if (_rd16(bmp) != 1) { bmp.close(); return false; } // planes
  uint16_t depth = _rd16(bmp); // 16 или 24
  uint32_t comp  = _rd32(bmp); // 0=BI_RGB, 3=BI_BITFIELDS (часто для 16-bit)

  if (!((comp == 0) || (comp == 3 && depth == 16))) { bmp.close(); return false; }
  if (!(depth == 24 || depth == 16)) { bmp.close(); return false; }

  bool flip = true;
  if (h < 0) { h = -h; flip = false; }

  // row size aligned to 4 bytes
  uint32_t rowSize = ((depth * (uint32_t)w + 31) / 32) * 4;

  // clip check (быстрый)
  int16_t x2 = x + (int16_t)w - 1;
  int16_t y2 = y + (int16_t)h - 1;
  if (x > (int16_t)tft.width()-1 || y > (int16_t)tft.height()-1 || x2 < 0 || y2 < 0) {
    bmp.close(); return true;
  }

  // line buffer (экран 160 ширина)
  static uint16_t line[160];

  tft.startWrite();

  for (int32_t row = 0; row < h; row++) {
    int32_t bmpRow = flip ? (h - 1 - row) : row;
    uint32_t pos = dataOff + (uint32_t)bmpRow * rowSize;
    if (!bmp.seek(pos)) { tft.endWrite(); bmp.close(); return false; }

    int16_t yy = y + (int16_t)row;
    if (yy < 0 || yy >= (int16_t)tft.height()) continue;

    int32_t cols = w;
    if (cols > 160) cols = 160;

    if (depth == 24) {
      for (int32_t col = 0; col < cols; col++) {
        uint8_t b = bmp.read();
        uint8_t g = bmp.read();
        uint8_t r = bmp.read();
        line[col] = tft.color565(r, g, b);
      }
    } else { // 16-bit
      for (int32_t col = 0; col < cols; col++) {
        line[col] = _rd16(bmp);
      }
    }

    int16_t xx = x;
    int16_t start = 0;
    int16_t drawW = (int16_t)w;

    if (xx < 0) { start = -xx; drawW += xx; xx = 0; }
    if (xx + drawW > (int16_t)tft.width()) drawW = (int16_t)tft.width() - xx;
    if (drawW <= 0) continue;

    // гарантированно работает на Adafruit_ST7735:
 tft.startWrite();
tft.setAddrWindow(xx, yy, drawW, 1);

for (int16_t i = 0; i < drawW; i++) {
  tft.writePixel(xx + i, yy, line[start + i]);
}

tft.endWrite();
  }

  tft.endWrite();
  bmp.close();
  return true;
}
