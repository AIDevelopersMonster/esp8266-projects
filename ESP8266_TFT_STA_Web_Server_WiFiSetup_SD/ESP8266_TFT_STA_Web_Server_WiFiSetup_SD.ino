#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "wifi_provision.h"
#include "app_routes.h"
#include "sd_test.h"
#include "sd_browser.h"   // он сам тянет img_draw.h

// ===== TFT pins =====
#define TFT_CS   D2
#define TFT_DC   D1
#define TFT_RST  -1

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nBOOT OK");

  SPI.begin();
  SPI.setFrequency(4000000);

  tft.initR(INITR_BLACKTAB);   // если надо — GREENTAB / REDTAB
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  SD_init(tft);
  delay(1500);  // чтобы увидеть "SD OK/FAIL"
  // 1) Wi-Fi provisioning: AP first time, then STA
  bool staReady = ensureWiFi(server, tft);

  // 2) If STA is ready -> start main app (router control)
  if (staReady) {
    setupAppRoutes(server, tft);
    registerSdBrowserRoutes();
    server.begin();

    Serial.print("APP IP: ");
    Serial.println(WiFi.localIP());
  }

  // Если staReady==false, значит сейчас открыт AP портал,
  // server уже запущен внутри ensureWiFi().
}

void loop() {
  server.handleClient();
  wifiResetButtonPoll();   // удержать 3 сек -> сброс + рестарт
}