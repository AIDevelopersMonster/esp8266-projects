#pragma once
#include <ESP8266WebServer.h>
#include <Adafruit_ST7735.h>

// ---- “более похожие на знак” стрелки ----
static inline uint16_t blueRoad(Adafruit_ST7735& tft) {
  return tft.color565(0, 80, 200);
}

static inline void signLeft(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(80, 64, 55, blueRoad(tft));

  // тело стрелки
  tft.fillRect(58, 58, 48, 12, ST77XX_WHITE);
  // голова
  tft.fillTriangle(48, 64, 62, 50, 62, 78, ST77XX_WHITE);
}

static inline void signRight(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(80, 64, 55, blueRoad(tft));
  tft.fillRect(54, 58, 48, 12, ST77XX_WHITE);
  tft.fillTriangle(112, 64, 98, 50, 98, 78, ST77XX_WHITE);
}

static inline void signBack(Adafruit_ST7735& tft) {
  // “назад/разворот” – упрощенно: стрелка вниз с крюком
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(80, 64, 55, blueRoad(tft));
  // вертикальное тело
  tft.fillRect(74, 40, 12, 40, ST77XX_WHITE);
  // голова вниз
  tft.fillTriangle(80, 88, 62, 70, 98, 70, ST77XX_WHITE);
  // “крюк” влево
  tft.fillRect(52, 40, 34, 12, ST77XX_WHITE);
  tft.fillTriangle(48, 46, 60, 34, 60, 58, ST77XX_WHITE);
}

static inline void signGreen(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);
  // зелёный круг (как разрешение)
  tft.fillCircle(80, 64, 55, tft.color565(0, 160, 60));
  // белая стрелка вверх
  tft.fillRect(74, 48, 12, 40, ST77XX_WHITE);
  tft.fillTriangle(80, 32, 60, 56, 100, 56, ST77XX_WHITE);
}

static inline void signStop(Adafruit_ST7735& tft) {
  tft.fillScreen(ST77XX_BLACK);
  // красный круг (запрещающий)
  tft.fillCircle(80, 64, 55, tft.color565(200, 0, 0));
  // белый крест
  for (int i = -22; i <= 22; i++) {
    tft.drawLine(80-22, 64-22+i, 80+22, 64+22+i, ST77XX_WHITE);
    tft.drawLine(80-22, 64+22-i, 80+22, 64-22-i, ST77XX_WHITE);
  }
}

static inline String controlPage() {
  return R"rawliteral(
  <!doctype html><html><head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>TFT Control</title>
    <style>
      body{margin:0;font-family:system-ui,Arial;background:#0b1220;color:#eaf0ff;
           display:flex;align-items:center;justify-content:center;min-height:100vh}
      .card{width:min(720px,94vw);background:#121a2b;border:1px solid rgba(255,255,255,.10);
            border-radius:18px;padding:18px;box-shadow:0 12px 30px rgba(0,0,0,.35)}
      h2{margin:6px 0 14px 0}
      .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
      button{
        padding:16px 10px;border-radius:14px;border:1px solid rgba(255,255,255,.12);
        background:#0b1220;color:#eaf0ff;font-size:20px;font-weight:800;cursor:pointer
      }
      button:active{transform:scale(.99)}
      .row{display:flex;gap:10px;margin-top:10px}
      .row button{flex:1}
      .go{background:#0f7f3b;border:none}
      .stop{background:#b00020;border:none}
      .clear{background:#2a3246;border:none}
      .hint{opacity:.75;font-size:13px;margin-top:12px}
    </style>
  </head><body>
    <div class="card">
      <h2>🚦 TFT Traffic Panel</h2>

      <div class="grid">
        <button onclick="hit('/left')">⬅</button>
        <button onclick="hit('/right')">➡</button>
        <button onclick="hit('/back')">↩</button>
      </div>

      <div class="row">
        <button class="go" onclick="hit('/go')">🟢 GO</button>
        <button class="stop" onclick="hit('/stop')">❌ STOP</button>
      </div>

      <div class="row">
        <button class="clear" onclick="hit('/clear')">Clear</button>
        <button class="clear" onclick="location.href='/'">Reload</button>
      </div>

      <div class="hint">Works in router (STA) mode. Open this page from phone or PC.</div>
    </div>

    <script>
      async function hit(p){
        try{ await fetch(p); }catch(e){}
      }
    </script>
  </body></html>
  )rawliteral";
}

static inline void setupAppRoutes(ESP8266WebServer& server, Adafruit_ST7735& tft) {
  server.on("/", [&](){ server.send(200, "text/html", controlPage()); });

  server.on("/left",  [&](){ signLeft(tft);  server.send(200, "text/plain", "OK"); });
  server.on("/right", [&](){ signRight(tft); server.send(200, "text/plain", "OK"); });
  server.on("/back",  [&](){ signBack(tft);  server.send(200, "text/plain", "OK"); });

  server.on("/go",    [&](){ signGreen(tft); server.send(200, "text/plain", "OK"); });
  server.on("/stop",  [&](){ signStop(tft);  server.send(200, "text/plain", "OK"); });

  server.on("/clear", [&](){
    tft.fillScreen(ST77XX_BLACK);
    server.send(200, "text/plain", "CLEARED");
  });
}