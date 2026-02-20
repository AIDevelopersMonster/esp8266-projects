#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Adafruit_ST7735.h>

// ===== EEPROM layout =====
static const int EEPROM_SIZE = 96;
static const int SSID_ADDR   = 0;
static const int PASS_ADDR   = 32;

// ===== AP setup network =====
static const char* SETUP_AP_SSID = "ESP-Setup";
// без пароля — чтобы точно виделось. Если хочешь пароль, добавим позже.

// ---- helpers ----
static inline void eepromClearAll() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  EEPROM.end();
}

// ===== Reset Wi-Fi by button =====
// Кнопка на D0 (GPIO16) -> GND. Используем INPUT_PULLUP.
#ifndef WIFI_RESET_BTN
  #define WIFI_RESET_BTN D0 //GPIO16
#endif
#ifndef WIFI_RESET_HOLD_MS
  #define WIFI_RESET_HOLD_MS 3000  // удержание 3 секунды
#endif

// Проверка удержания при старте (вызов внутри ensureWiFi)
static inline bool wifiResetPressedLong() {
  pinMode(WIFI_RESET_BTN, INPUT_PULLUP);
  if (digitalRead(WIFI_RESET_BTN) != LOW) return false;

  unsigned long t0 = millis();
  while (digitalRead(WIFI_RESET_BTN) == LOW) {
    delay(10);
    if (millis() - t0 >= WIFI_RESET_HOLD_MS) return true;
    yield();
  }
  return false;
}

// (Опционально) вызывать в loop(), чтобы сброс работал в любой момент
static inline void wifiResetButtonPoll() {
  static unsigned long pressStart = 0;
  pinMode(WIFI_RESET_BTN, INPUT_PULLUP);

  if (digitalRead(WIFI_RESET_BTN) == LOW) {
    if (pressStart == 0) pressStart = millis();
    if (millis() - pressStart >= WIFI_RESET_HOLD_MS) {
      eepromClearAll();
      delay(200);
      ESP.restart();
    }
  } else {
    pressStart = 0;
  }
}


static inline void eepromWriteString(int addr, const String& s, int maxLen) {
  // полностью затираем область (важно, чтобы не было "хвостов"!)
  for (int i = 0; i < maxLen; i++) {
    char c = (i < (int)s.length()) ? s[i] : 0;
    EEPROM.write(addr + i, c);
  }
}

static inline String eepromReadString(int addr, int maxLen) {
  char buf[64];
  int n = (maxLen < 63) ? maxLen : 63;
  for (int i = 0; i < n; i++) {
    uint8_t v = EEPROM.read(addr + i);
    if (v == 0xFF || v == 0) { buf[i] = 0; break; }
    buf[i] = (char)v;
  }
  buf[n] = 0;
  String out(buf);
  out.trim();
  return out;
}

static inline void saveWiFiCreds(const String& ssid, const String& pass) {
  EEPROM.begin(EEPROM_SIZE);
  eepromWriteString(SSID_ADDR, ssid, 32);
  eepromWriteString(PASS_ADDR, pass, 32);
  EEPROM.commit();
}

static inline void loadWiFiCreds(String& ssid, String& pass) {
  EEPROM.begin(EEPROM_SIZE);
  ssid = eepromReadString(SSID_ADDR, 32);
  pass = eepromReadString(PASS_ADDR, 32);
}

// ---- UI pages ----
static inline String wifiSetupPage() {
  return R"rawliteral(
  <!doctype html><html><head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Wi-Fi Setup</title>
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
  </head><body>
    <div class="card">
      <h2>Wi-Fi Setup</h2>

      <label>Available networks</label>
      <select id="nets"><option>Press “Scan Wi-Fi”…</option></select>

      <div class="row">
        <button type="button" class="secondary" onclick="scan()">Scan Wi-Fi</button>
        <button type="button" class="secondary" onclick="useSel()">Use</button>
      </div>

      <form action="/save" method="get">
        <label>SSID</label>
        <input id="ssid" name="s" placeholder="MyHomeWiFi" autocapitalize="none">

        <label>Password <small>(leave empty for open networks)</small></label>
        <input id="pass" name="p" type="password" placeholder="********" autocomplete="off">

        <button type="submit" style="margin-top:14px">Save & Restart</button>
      </form>

      <div class="row">
        <button type="button" class="secondary" onclick="togglePass()">Show/Hide password</button>
        <button type="button" class="secondary" onclick="location.href='/reset'">Reset Wi-Fi</button>
      </div>

      <div class="hint">
        1) Connect to <b>ESP-Setup</b><br>
        2) Open <b>192.168.4.1</b><br>
        3) Scan → choose SSID → enter password → Save
      </div>
    </div>

    <script>
      async function scan(){
        const sel = document.getElementById('nets');
        sel.innerHTML = '<option>Scanning…</option>';
        try{
          const r = await fetch('/scan');
          const arr = await r.json();
          sel.innerHTML = '';
          if(!arr.length){ sel.innerHTML = '<option>No networks found</option>'; return; }
          arr.sort((a,b)=>b.rssi-a.rssi);
          for(const n of arr){
            const opt = document.createElement('option');
            const lock = n.enc ? '🔒' : '🟢';
            opt.value = n.ssid;
            opt.textContent = `${lock} ${n.ssid} (${n.rssi} dBm)`;
            sel.appendChild(opt);
          }
        }catch(e){
          sel.innerHTML = '<option>Scan error</option>';
        }
      }
      function useSel(){
        const sel = document.getElementById('nets');
        document.getElementById('ssid').value = sel.value || '';
      }
      function togglePass(){
        const p = document.getElementById('pass');
        p.type = (p.type === 'password') ? 'text' : 'password';
      }
    </script>
  </body></html>
  )rawliteral";
}

// ---- main entry: ensure WiFi ----
// returns true when STA connected and app can start.
// returns false when AP portal is running (setup mode).
static inline bool ensureWiFi(ESP8266WebServer& server, Adafruit_ST7735& tft) {
  // --- Сброс Wi-Fi по кнопке (D6, удерживать 3 сек) ---
  if (wifiResetPressedLong()) {
    eepromClearAll();

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.println("WiFi RESET");
    tft.println("Restarting...");

    delay(800);
    ESP.restart();
  }


  String ssid, pass;
  loadWiFiCreds(ssid, pass);

  // Try STA if we have creds
  if (ssid.length() > 1) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    // MGTS/некоторые роутеры — дольше коннект, ставим 20 сек
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
      delay(300);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());

      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      tft.setCursor(0, 0);
      tft.print("IP: ");
      tft.println(WiFi.localIP());

      return true;
    }

    Serial.println("WiFi connect failed -> starting AP setup");
  }

  // Start AP portal (always)
  WiFi.mode(WIFI_AP_STA);  // важно: чтобы работал Wi-Fi scan
  WiFi.softAP(SETUP_AP_SSID);

  Serial.println("Starting AP: ESP-Setup");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.println("WiFi Setup Mode");
  tft.print("AP: "); tft.println(SETUP_AP_SSID);
  tft.print("IP: "); tft.println(WiFi.softAPIP());

  server.on("/", [&](){ server.send(200, "text/html", wifiSetupPage()); });

  server.on("/scan", [&](){
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
      if (i) json += ",";
      String s = WiFi.SSID(i);
      s.replace("\\", "\\\\");
      s.replace("\"", "\\\"");
      json += "{\"ssid\":\"" + s + "\",\"rssi\":" + String(WiFi.RSSI(i)) +
              ",\"enc\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? 1 : 0) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/save", [&](){
    String s = server.arg("s"); s.trim();
    String p = server.arg("p"); p.trim();
    saveWiFiCreds(s, p);
    server.send(200, "text/plain", "Saved! Restarting...");
    delay(1200);
    ESP.restart();
  });

  // reset creds from browser
  server.on("/reset", [&](){
    eepromClearAll();
    server.send(200, "text/plain", "WiFi cleared. Restarting...");
    delay(800);
    ESP.restart();
  });

  server.begin();
  return false;
}
