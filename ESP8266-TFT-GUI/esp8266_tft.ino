#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   D2
#define TFT_DC   D1
#define TFT_RST  -1   // RST дисплея подключи к RST платы!

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

String input = "";

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TFT Serial Control Ready ===");

  SPI.begin();
  SPI.setFrequency(4000000);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextWrap(false);

  Serial.println("Команды:");
  Serial.println("FILL r g b");
  Serial.println("TEXT x y size r g b message");
  Serial.println("Пример: TEXT 10 30 2 255 255 0 Hello");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processCommand(input);
      input = "";
    } else {
      input += c;
    }
  }
}

void processCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  Serial.print("CMD: ");
  Serial.println(cmd);

  // ---- FILL ----
  if (cmd.startsWith("FILL")) {
    int r, g, b;
    sscanf(cmd.c_str(), "FILL %d %d %d", &r, &g, &b);

    uint16_t color = tft.color565(r, g, b);
    tft.fillScreen(color);

    Serial.println("OK: Screen filled");
  }

  // ---- TEXT ----
  else if (cmd.startsWith("TEXT")) {
    int x, y, size;
    int r, g, b;

    char msg[100];

    sscanf(cmd.c_str(),
           "TEXT %d %d %d %d %d %d %[^\n]",
           &x, &y, &size,
           &r, &g, &b,
           msg);

    tft.setCursor(x, y);
    tft.setTextSize(size);
    tft.setTextColor(tft.color565(r, g, b), ST77XX_BLACK);

    tft.print(msg);

    Serial.println("OK: Text printed");
  }

  else {
    Serial.println("❌ Unknown command");
  }
}
