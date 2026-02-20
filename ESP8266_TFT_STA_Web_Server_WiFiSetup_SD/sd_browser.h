#pragma once
#include <SD.h>
#include <ESP8266WebServer.h>

#include "img_draw.h"

// объявлен в .ino
extern ESP8266WebServer server;

// ----------------- utils -----------------
static bool sd_isSafePath(const String& p) {
  if (!p.startsWith("/")) return false;
  if (p.indexOf("..") >= 0) return false;
  return true;
}

static String sd_guessMime(const String& path) {
  String p = path; p.toLowerCase();
  if (p.endsWith(".html")) return "text/html";
  if (p.endsWith(".css"))  return "text/css";
  if (p.endsWith(".js"))   return "application/javascript";
  if (p.endsWith(".png"))  return "image/png";
  if (p.endsWith(".jpg") || p.endsWith(".jpeg")) return "image/jpeg";
  if (p.endsWith(".bmp"))  return "image/bmp";
  if (p.endsWith(".gif"))  return "image/gif";
  if (p.endsWith(".json")) return "application/json";
  if (p.endsWith(".txt"))  return "text/plain";
  return "application/octet-stream";
}

// ----------------- API: list dir -----------------
// GET /api/list?dir=/roadsigns
// returns: [{"name":"/roadsigns/a.bmp","dir":false,"size":1234}, ...]
static void sd_handleApiList() {
  String dir = server.arg("dir");
  if (dir == "") dir = "/";
  if (!sd_isSafePath(dir)) { server.send(400, "text/plain", "Bad dir"); return; }

  File d = SD.open(dir);
  if (!d || !d.isDirectory()) { server.send(404, "text/plain", "No dir"); return; }

  String out = "[";
  bool first = true;

 File f = d.openNextFile();
while (f) {
  String nm = String(f.name());   // часто тут только "file.bmp"

  // Сделаем имя относительным (уберём ведущие слэши, если вдруг есть)
  while (nm.startsWith("/")) nm.remove(0, 1);

  // Соберём полный путь из текущего dir
  String full = dir;
  if (!full.endsWith("/")) full += "/";
  full += nm;                     // -> "/roadsigns/znak.bmp"

  if (!first) out += ",";
  first = false;

  out += "{\"name\":\"" + full + "\",\"dir\":" +
         String(f.isDirectory() ? "true" : "false") +
         ",\"size\":" + String((uint32_t)f.size()) + "}";

  f = d.openNextFile();
}
  out += "]";
  server.send(200, "application/json", out);
}

// ----------------- SD file streaming -----------------
// GET /sd?path=/roadsigns/a.bmp
static void sd_handleGetFile() {
  String path = server.arg("path");
  if (path == "") { server.send(400, "text/plain", "Missing path"); return; }
  if (!sd_isSafePath(path)) { server.send(400, "text/plain", "Bad path"); return; }
  if (!SD.exists(path)) { server.send(404, "text/plain", "Not found"); return; }

  File f = SD.open(path, FILE_READ);
  if (!f) { server.send(500, "text/plain", "Open error"); return; }

  server.streamFile(f, sd_guessMime(path));
  f.close();
}

// ----------------- API: show on TFT -----------------
// GET /api/show?file=/roadsigns/a.bmp
// optional: &full=1  -> draw at (0,0) for 160x128 canvas BMP
static void sd_handleApiShow() {
  String file = server.arg("file");
  if (!sd_isSafePath(file)) { server.send(400, "text/plain", "Bad file"); return; }
  if (!SD.exists(file))  { server.send(404, "text/plain", "Not found"); return; }

  int16_t x = 16, y = 0;            // центр 128x128 на 160x128
  if (server.arg("full") == "1") {  // если файл 160x128
    x = 0; y = 0;
  }

  bool ok = drawBmpFromSD(file.c_str(), x, y);
  server.send(ok ? 200 : 500, "text/plain", ok ? "OK" : "DRAW_ERR");
}

// ----------------- UI page -----------------
static void sd_handleFilesPage() {
  String html =
    "<!doctype html><html><head><meta charset='utf-8'/>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'/>"
    "<title>SD Browser</title>"
    "<style>"
    "body{font-family:sans-serif;padding:12px}"
    "a{display:block;padding:6px 0;text-decoration:none}"
    "button{margin:4px;padding:8px}"
    "</style></head><body>"
    "<h3>SD Browser</h3>"
    "<div>"
    "<button onclick=\"openDir('/')\">/</button>"
    "<button onclick=\"openDir('/roadsigns')\">/roadsigns</button>"
    "<button onclick=\"openDir('/roadsigns_test')\">/roadsigns_test</button>"
    "<button onclick=\"openDir('/qr')\">/qr</button>"
    "<button onclick=\"openDir('/apriltag')\">/apriltag</button>"
    "<button onclick=\"openDir('/config')\">/config</button>"
    "</div>"
    "<p id='cur'></p>"
    "<div id='list'></div>"
    "<script>"
    "var cur='/';"

    "function escHtml(s){"
    "  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')"
    "    .replace(/\"/g,'&quot;').replace(/'/g,'&#39;');"
    "}"

    "function openDir(d){"
    "  cur=d;"
    "  document.getElementById('cur').textContent='Dir: '+d;"
    "  fetch('/api/list?dir='+encodeURIComponent(d))"
    "    .then(function(r){return r.json();})"
    "    .then(function(arr){"
    "      var out='';"
    "      if(d!='/'){"
    "        var p=d.replace(/\\/+$/,'');"
    "        var up=p.substring(0,p.lastIndexOf('/'));"
    "        if(up==='') up='/';"
    "        out += '<a href=\"#\" onclick=\"openDir(\\''+up+'\\');return false;\">⬅ ..</a>';"
    "      }"
    "      for(var i=0;i<arr.length;i++){"
    "        var x=arr[i];"
    "        if(x.dir){"
    "          out += '<a href=\"#\" onclick=\"openDir(\\''+x.name+'\\');return false;\">📁 '+escHtml(x.name)+'</a>';"
    "        } else {"
    "          out += '<a target=\"_blank\" href=\"/sd?path='+encodeURIComponent(x.name)+'\">📄 '+escHtml(x.name)+'</a>';"
    "          out += ' <button onclick=\"fetch(\\'/api/show?file='+encodeURIComponent(x.name)+'\\');\">SHOW</button><br/>';"
    "        }"
    "      }"
    "      document.getElementById('list').innerHTML=out;"
    "    });"
    "}"

    "openDir(cur);"
    "</script></body></html>";

  server.send(200, "text/html", html);
}

// ----------------- public API -----------------
static void registerSdBrowserRoutes() {
  // UI
  server.on("/files", HTTP_GET, sd_handleFilesPage);

  // API
  server.on("/api/list", HTTP_GET, sd_handleApiList);
  server.on("/api/show", HTTP_GET, sd_handleApiShow);

  // File streaming
  server.on("/sd", HTTP_GET, sd_handleGetFile);
}
