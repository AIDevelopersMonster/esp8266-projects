#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ===== TFT PINS =====
#define TFT_CS   D2
#define TFT_DC   D1
#define TFT_RST  -1

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
ESP8266WebServer server(80);

// ===== EEPROM =====
#define EEPROM_SIZE 96

String ssidStored = "";
String passStored = "";

// ---------- EEPROM HELPERS ----------
void saveWiFi(String s, String p) {
  EEPROM.begin(EEPROM_SIZE);

  for (int i = 0; i < 32; i++)
    EEPROM.write(i, i < s.length() ? s[i] : 0);

  for (int i = 0; i < 32; i++)
    EEPROM.write(32 + i, i < p.length() ? p[i] : 0);

  EEPROM.commit();
}

void loadWiFi() {
  EEPROM.begin(EEPROM_SIZE);

  char s[33], p[33];

  for (int i = 0; i < 32; i++) s[i] = EEPROM.read(i);
  for (int i = 0; i < 32; i++) p[i] = EEPROM.read(32 + i);

  s[32] = 0;
  p[32] = 0;

  // если EEPROM пустая (0xFF) → строка пустая
  if ((uint8_t)s[0] == 0xFF) s[0] = 0;

  ssidStored = String(s);
  passStored = String(p);

  ssidStored.trim();
  passStored.trim();
}

// ---------- TFT ICONS ----------
void drawBlueCircle() {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(80, 64, 50, ST77XX_BLUE);
}

void arrowLeft() {
  drawBlueCircle();
  tft.fillTriangle(60, 64, 100, 40, 100, 88, ST77XX_WHITE);
}

void arrowRight() {
  drawBlueCircle();
  tft.fillTriangle(100, 64, 60, 40, 60, 88, ST77XX_WHITE);
}

void arrowBack() {
  drawBlueCircle();
  tft.fillTriangle(80, 30, 50, 80, 110, 80, ST77XX_WHITE);
}

void greenArrow() {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillTriangle(80, 30, 50, 80, 110, 80, ST77XX_GREEN);
}

void redCross() {
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(55, 55, 50, 10, ST77XX_RED);
  tft.fillRect(75, 35, 10, 50, ST77XX_RED);
}

// ---------- WEB PAGES ----------
String wifiPage() {
  return R"rawliteral(
  <!doctype html>
  <html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>ESP WiFi Setup</title>
    <style>
      body{margin:0;font-family:system-ui,Arial;background:#0b1220;color:#eaf0ff;
           display:flex;align-items:center;justify-content:center;min-height:100vh}
      .card{width:min(520px,92vw);background:#121a2b;border:1px solid rgba(255,255,255,.10);
            border-radius:18px;padding:18px;box-shadow:0 12px 30px rgba(0,0,0,.35)}
      h2{margin:6px 0 14px 0;font-size:22px}
      label{display:block;margin:12px 0 6px 0;opacity:.9;font-size:14px}
      input,select,button{
        width:100%;box-sizing:border-box;padding:14px 14px;font-size:18px;border-radius:14px;
        border:1px solid rgba(255,255,255,.14);background:#0b1220;color:#eaf0ff;outline:none
      }
      button{border:none;background:#2b59ff;color:white;font-weight:800;cursor:pointer}
      button.secondary{background:#2a3246}
      .row{display:flex;gap:10px;margin-top:12px}
      .hint{margin-top:12px;font-size:13px;opacity:.75;line-height:1.35}
      small{opacity:.75}
    </style>
  </head>
  <body>
    <div class="card">
      <h2>Wi-Fi Setup</h2>

      <label>Available networks</label>
      <select id="networks">
        <option>Press “Scan Wi-Fi”…</option>
      </select>

      <div class="row">
        <button type="button" class="secondary" onclick="scan()">Scan Wi-Fi</button>
        <button type="button" class="secondary" onclick="useSelected()">Use</button>
      </div>

      <form action="/save" method="get">
        <label>SSID</label>
        <input id="ssid" name="s" placeholder="Choose from list or type вручную" autocapitalize="none">

        <label>Password <small>(leave empty for open networks)</small></label>
        <input id="pass" name="p" type="password" placeholder="********" autocomplete="off">

        <button type="submit" style="margin-top:14px">Save & Restart</button>
      </form>

      <div class="row">
        <button type="button" class="secondary" onclick="togglePass()">Show/Hide password</button>
      </div>

      <div class="hint">
        Подключись к сети <b>ESP-Setup</b>, открой <b>192.168.4.1</b>.<br>
        Нажми <b>Scan Wi-Fi</b>, выбери сеть, введи пароль и сохрани.
      </div>
    </div>

    <script>
      async function scan(){
        const sel = document.getElementById('networks');
        sel.innerHTML = '<option>Scanning…</option>';
        try{
          const r = await fetch('/scan');
          const arr = await r.json();
          sel.innerHTML = '';
          if(!arr.length){
            sel.innerHTML = '<option>No networks found</option>';
            return;
          }
          // сортировка: сильнее сигнал сверху
          arr.sort((a,b)=>b.rssi-a.rssi);
          for(const n of arr){
            const opt = document.createElement('option');
            const lock = n.enc ? '🔒' : '🟢';
            opt.value = n.ssid;
            opt.textContent = `${lock} ${n.ssid}  (${n.rssi} dBm)`;
            sel.appendChild(opt);
          }
        }catch(e){
          sel.innerHTML = '<option>Scan error</option>';
        }
      }

      function useSelected(){
        const sel = document.getElementById('networks');
        const ssid = sel.value || '';
        document.getElementById('ssid').value = ssid;
      }

      function togglePass(){
        const p = document.getElementById('pass');
        p.type = (p.type === 'password') ? 'text' : 'password';
      }
    </script>
  </body>
  </html>
  )rawliteral";
}



String controlPage() {
  return R"rawliteral(
  <html>
  <body style="font-family:Arial;text-align:center;background:#0b1220;color:white">
    <h2>TFT Traffic Control</h2>

    <button onclick="fetch('/left')"  style="width:120px;height:70px;font-size:30px">⬅</button>
    <button onclick="fetch('/right')" style="width:120px;height:70px;font-size:30px">➡</button>
    <button onclick="fetch('/back')"  style="width:120px;height:70px;font-size:30px">↩</button>

    <br><br>

    <button onclick="fetch('/go')" style="background:green;width:160px;height:70px;font-size:22px">
      🟢 GO
    </button>

    <button onclick="fetch('/stop')" style="background:red;width:160px;height:70px;font-size:22px">
      ❌ STOP
    </button>

    <br><br>

    <button onclick="fetch('/clear')" style="width:200px;height:50px;font-size:18px">
      Clear Screen
    </button>

  </body>
  </html>
  )rawliteral";
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\nBOOT OK");

  SPI.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  loadWiFi();

  bool connected = false;

  // ----- TRY ROUTER MODE -----
  if (ssidStored.length() > 1) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssidStored);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidStored.c_str(), passStored.c_str());

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
      delay(300);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.println("\nConnected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  }

  // ----- FALLBACK: ALWAYS START AP -----
  if (!connected) {
    Serial.println("\nStarting AP: ESP-Setup");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP-Setup");

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", []() {
      server.send(200, "text/html", wifiPage());
    });
server.on("/scan", []() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");
    json += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(WiFi.RSSI(i)) +
            ",\"enc\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? 1 : 0) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
});
    server.on("/save", []() {
      String s = server.arg("s");
      String p = server.arg("p");

      saveWiFi(s, p);

      server.send(200, "text/plain", "Saved! Restarting...");
      delay(1500);
      ESP.restart();
    });

    server.begin();
    return;
  }

  // ----- CONTROL MODE -----
  server.on("/", []() {
    server.send(200, "text/html", controlPage());
  });

  server.on("/left", []() { arrowLeft(); server.send(200, "text/plain", "OK"); });
  server.on("/right", []() { arrowRight(); server.send(200, "text/plain", "OK"); });
  server.on("/back", []() { arrowBack(); server.send(200, "text/plain", "OK"); });

  server.on("/go", []() { greenArrow(); server.send(200, "text/plain", "OK"); });
  server.on("/stop", []() { redCross(); server.send(200, "text/plain", "OK"); });

  server.on("/clear", []() {
    tft.fillScreen(ST77XX_BLACK);
    server.send(200, "text/plain", "CLEARED");
  });

  server.begin();
}

// ---------- LOOP ----------
void loop() {
  server.handleClient();
}
