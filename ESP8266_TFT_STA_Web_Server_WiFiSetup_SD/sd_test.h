#pragma once
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#ifndef SD_CS
  #define SD_CS D8   // CS для SD (рекомендуется D8/GPIO15)
#endif

static bool SD_ready = false;

static inline void sd_showStatus(Adafruit_ST7735 &tft, bool ok) {
  tft.fillScreen(ok ? ST77XX_BLACK : ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  tft.setCursor(10, 20);
  tft.setTextColor(ok ? ST77XX_GREEN : ST77XX_RED, ST77XX_BLACK);
  tft.println(ok ? "SD OK" : "SD FAIL");
}

static inline bool SD_init(Adafruit_ST7735 &tft) {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);   // SD не выбрана

  Serial.println("\nSD init...");

  if (!SD.begin(SD_CS)) {
    Serial.println("SD init FAILED");
    sd_showStatus(tft, false);
    SD_ready = false;
    return false;
  }

  Serial.println("SD init OK");
  sd_showStatus(tft, true);
  SD_ready = true;

  // Листинг корня
  File root = SD.open("/");
  if (!root) {
    Serial.println("SD open / FAILED");
    return true; // SD есть, но корень не открылся (редко)
  }

  while (true) {
    File f = root.openNextFile();
    if (!f) break;

    Serial.print("FILE: ");
    Serial.print(f.name());
    Serial.print("  SIZE: ");
    Serial.println((unsigned long)f.size());
    f.close();
  }
  root.close();

  return true;
}