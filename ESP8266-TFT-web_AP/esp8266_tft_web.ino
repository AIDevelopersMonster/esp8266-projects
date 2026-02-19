#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ===== TFT pins (NodeMCU / D1 mini) =====
#define TFT_CS   D2   // GPIO4
#define TFT_DC   D1   // GPIO5
#define TFT_RST  -1   // RST дисплея на RST платы (рекомендовано)

// ===== Wi-Fi AP settings =====
const char* AP_SSID = "ESP8266-TFT";
const char* AP_PASS = "12345678"; // минимум 8 символов, можно "" (открытая сеть)

ESP8266WebServer server(80);
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// Текущее состояние
String lastText = "Hello!";
int lastX = 10, lastY = 30, lastSize = 2;
uint8_t lastTR = 255, lastTG = 255, lastTB = 0;   // цвет текста
uint8_t lastBR = 0, lastBG = 0, lastBB = 0;       // фон

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

static bool parseHexColor(const String& hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  // ожидаем "#RRGGBB"
  if (hex.length() != 7 || hex[0] != '#') return false;
  auto h2i = [](char c)->int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
  };
  int a = h2i(hex[1]), b1 = h2i(hex[2]);
  int c1 = h2i(hex[3]), d = h2i(hex[4]);
  int e = h2i(hex[5]), f = h2i(hex[6]);
  if (a < 0 || b1 < 0 || c1 < 0 || d < 0 || e < 0 || f < 0) return false;
  r = (a << 4) | b1;
  g = (c1 << 4) | d;
  b = (e << 4) | f;
  return true;
}

void drawScreen() {
  // фон
  tft.fillScreen(rgb565(lastBR, lastBG, lastBB));

  // текст
  tft.setTextWrap(false);
  tft.setCursor(lastX, lastY);
  tft.setTextSize(lastSize);
  // фон текста = фон экрана, чтобы "переписывалось" аккуратно
  tft.setTextColor(rgb565(lastTR, lastTG, lastTB), rgb565(lastBR, lastBG, lastBB));
  tft.print(lastText);
}

String pageHtml() {
  // Простая страница управления
  String html =
    "<!doctype html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>ESP8266 TFT</title>"
    "<style>"
    "body{font-family:system-ui,Arial;margin:16px;background:#0b1220;color:#eaf0ff}"
    ".card{max-width:720px;margin:auto;background:#121a2b;border:1px solid rgba(255,255,255,.08);"
    "border-radius:16px;padding:16px;box-shadow:0 10px 30px rgba(0,0,0,.35)}"
    "h1{margin:0 0 10px 0;font-size:22px}"
    "label{display:block;margin:10px 0 6px;opacity:.9}"
    "input,button{width:100%;padding:12px;border-radius:12px;border:1px solid rgba(255,255,255,.12);"
    "background:#0b1220;color:#eaf0ff;font-size:16px}"
    ".row{display:flex;gap:10px}"
    ".row>div{flex:1}"
    "button{cursor:pointer;background:#2b59ff;border:none;font-weight:700}"
    "button:hover{filter:brightness(1.05)}"
    ".hint{opacity:.7;font-size:13px;margin-top:10px}"
    "</style></head><body><div class='card'>"
    "<h1>ESP8266 TFT Control (AP + WEB)</h1>"
    "<form action='/set' method='get'>"

    "<label>Текст</label>"
    "<input name='msg' value='" + lastText + "'>"

    "<div class='row'><div>"
    "<label>X</label><input type='number' name='x' min='0' max='200' value='" + String(lastX) + "'>"
    "</div><div>"
    "<label>Y</label><input type='number' name='y' min='0' max='200' value='" + String(lastY) + "'>"
    "</div><div>"
    "<label>Size</label><input type='number' name='s' min='1' max='6' value='" + String(lastSize) + "'>"
    "</div></div>"

    "<div class='row'><div>"
    "<label>Цвет текста</label>"
    "<input type='color' name='tc' value='#FFFF00'>"
    "</div><div>"
    "<label>Фон</label>"
    "<input type='color' name='bc' value='#000000'>"
    "</div></div>"

    "<div style='margin-top:14px'>"
    "<button type='submit'>Показать на TFT</button>"
    "</div>"
    "</form>"

    "<form action='/clear' method='get' style='margin-top:10px'>"
    "<button type='submit' style='background:#2a3246'>Очистить (чёрный)</button>"
    "</form>"

    "<div class='hint'>Подключись к Wi-Fi <b>" + String(AP_SSID) + "</b> и открой <b>http://192.168.4.1</b></div>"
    "</div></body></html>";

  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", pageHtml());
}

void handleClear() {
  lastBR = lastBG = lastBB = 0;
  lastText = "";
  drawScreen();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "OK");
}

void handleSet() {
  if (server.hasArg("msg")) lastText = server.arg("msg");
  if (server.hasArg("x")) lastX = server.arg("x").toInt();
  if (server.hasArg("y")) lastY = server.arg("y").toInt();
  if (server.hasArg("s")) lastSize = server.arg("s").toInt();

  // цвета из color input
  if (server.hasArg("tc")) {
    uint8_t r,g,b;
    if (parseHexColor(server.arg("tc"), r,g,b)) { lastTR=r; lastTG=g; lastTB=b; }
  }
  if (server.hasArg("bc")) {
    uint8_t r,g,b;
    if (parseHexColor(server.arg("bc"), r,g,b)) { lastBR=r; lastBG=g; lastBB=b; }
  }

  // ограничения
  if (lastSize < 1) lastSize = 1;
  if (lastSize > 6) lastSize = 6;
  if (lastX < 0) lastX = 0;
  if (lastY < 0) lastY = 0;

  drawScreen();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  SPI.begin();
  SPI.setFrequency(4000000);

  // TFT init (если не заведётся — поменяй на INITR_GREENTAB / INITR_REDTAB)
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // Wi-Fi AP
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();

  Serial.println();
  Serial.println(ok ? "AP started" : "AP failed");
  Serial.print("SSID: "); Serial.println(AP_SSID);
  Serial.print("IP:   "); Serial.println(ip);

  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("WiFi: ");
  tft.println(AP_SSID);
  tft.print("IP: ");
  tft.println(ip);

  // Web routes
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/clear", handleClear);
  server.begin();

  // начальный экран
  lastText = "ESP8266 TFT WEB";
  lastX = 10; lastY = 50; lastSize = 2;
  lastTR = 255; lastTG = 255; lastTB = 0;
  lastBR = 0; lastBG = 0; lastBB = 0;
  drawScreen();
}

void loop() {
  server.handleClient();
}
