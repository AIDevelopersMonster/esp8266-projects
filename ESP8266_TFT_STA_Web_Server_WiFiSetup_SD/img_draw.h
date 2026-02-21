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

// Рисует BMP по координатам (x,y). Поддержка 24-bit, 16-bit (RGB565) и 1-bit (indexed), без сжатия.
static bool drawBmpFromSD(const char* filename, int16_t x, int16_t y) {
  File bmp = SD.open(filename, FILE_READ);
  if (!bmp) return false;

  if (_rd16(bmp) != 0x4D42) { bmp.close(); return false; } // 'BM'
  (void)_rd32(bmp); // fileSize
  (void)_rd32(bmp); // reserved
  uint32_t dataOff = _rd32(bmp);

  uint32_t headerSize = _rd32(bmp); // DIB header size
  int32_t w = (int32_t)_rd32(bmp);
  int32_t h = (int32_t)_rd32(bmp);

  if (_rd16(bmp) != 1) { bmp.close(); return false; } // planes
  uint16_t depth = _rd16(bmp); // 1 / 16 / 24
  uint32_t comp  = _rd32(bmp); // 0=BI_RGB, 3=BI_BITFIELDS (часто для 16-bit)

  // Разрешаем:
  // - 24-bit BI_RGB
  // - 16-bit BI_RGB или BI_BITFIELDS
  // - 1-bit BI_RGB (indexed)
  bool ok =
    (depth == 24 && comp == 0) ||
    (depth == 16 && (comp == 0 || comp == 3)) ||
    (depth ==  1 && comp == 0);

  if (!ok) { bmp.close(); return false; }

  bool flip = true;
  if (h < 0) { h = -h; flip = false; }

  // clip check (быстрый)
  int16_t x2 = x + (int16_t)w - 1;
  int16_t y2 = y + (int16_t)h - 1;
  if (x > (int16_t)tft.width()-1 || y > (int16_t)tft.height()-1 || x2 < 0 || y2 < 0) {
    bmp.close(); return true;
  }

  // line buffer (экран 160 ширина)
  static uint16_t line[160];

  // Для 1bpp: читаем палитру (2 цвета) если она есть.
  // Палитра начинается сразу после DIB заголовка: offset = 14 + headerSize.
  // Каждый entry: 4 байта (B,G,R,0)
  uint16_t pal0 = ST77XX_WHITE;
  uint16_t pal1 = ST77XX_BLACK;
  if (depth == 1) {
    uint32_t palOff = 14 + headerSize;
    // Палитра должна быть до dataOff. Если места нет — используем дефолт.
    if (palOff + 8 <= dataOff) {
      if (bmp.seek(palOff)) {
        uint8_t b0 = bmp.read(), g0 = bmp.read(), r0 = bmp.read(); (void)bmp.read();
        uint8_t b1 = bmp.read(), g1 = bmp.read(), r1 = bmp.read(); (void)bmp.read();
        pal0 = tft.color565(r0, g0, b0);
        pal1 = tft.color565(r1, g1, b1);
      }
    }
  }

  tft.startWrite();

  if (depth == 24 || depth == 16) {
    // row size aligned to 4 bytes
    uint32_t rowSize = ((depth * (uint32_t)w + 31) / 32) * 4;

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

      tft.setAddrWindow(xx, yy, drawW, 1);
      for (int16_t i = 0; i < drawW; i++) {
        tft.writePixel(xx + i, yy, line[start + i]);
      }
    }

  } else {
    // ===== 1 bpp =====
    // bytes per row (без padding) = (w+7)/8
    // BMP padding до 4 байт
    uint32_t rowBytes = ((uint32_t)w + 7) / 8;
    uint32_t rowSize  = (rowBytes + 3) & ~3UL;

    // буфер под строку (для 160px достаточно 20 байт + padding)
    static uint8_t rowBuf[24];

    for (int32_t row = 0; row < h; row++) {
      int32_t bmpRow = flip ? (h - 1 - row) : row;
      uint32_t pos = dataOff + (uint32_t)bmpRow * rowSize;
      if (!bmp.seek(pos)) { tft.endWrite(); bmp.close(); return false; }

      int16_t yy = y + (int16_t)row;
      if (yy < 0 || yy >= (int16_t)tft.height()) continue;

      // ограничим w под 160, но читать строку всё равно нужно правильно.
      // Если w > 160, мы просто рисуем первые 160 пикселей.
      uint16_t drawCols = (w > 160) ? 160 : (uint16_t)w;

      // читаем минимум нужных байт строки, но не больше буфера
      uint32_t needBytes = ((uint32_t)drawCols + 7) / 8;
      if (needBytes > sizeof(rowBuf)) needBytes = sizeof(rowBuf);

      // читаем байты строки
      for (uint32_t i = 0; i < needBytes; i++) rowBuf[i] = (uint8_t)bmp.read();

      // преобразуем в line[]
      uint16_t col = 0;
      for (uint32_t bi = 0; bi < needBytes && col < drawCols; bi++) {
        uint8_t b = rowBuf[bi];
        for (int bit = 7; bit >= 0 && col < drawCols; bit--) {
          bool on = (b >> bit) & 1;
          line[col++] = on ? pal1 : pal0; // 1->pal1, 0->pal0
        }
      }

      int16_t xx = x;
      int16_t start = 0;
      int16_t drawW = (int16_t)drawCols;

      if (xx < 0) { start = -xx; drawW += xx; xx = 0; }
      if (xx + drawW > (int16_t)tft.width()) drawW = (int16_t)tft.width() - xx;
      if (drawW <= 0) continue;

      tft.setAddrWindow(xx, yy, drawW, 1);
      for (int16_t i = 0; i < drawW; i++) {
        tft.writePixel(xx + i, yy, line[start + i]);
      }
    }
  }

  tft.endWrite();
  bmp.close();
  return true;
}
