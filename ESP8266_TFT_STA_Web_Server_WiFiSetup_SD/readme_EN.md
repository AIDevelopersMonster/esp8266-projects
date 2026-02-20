---

# ESP8266 TFT Web Image Loader

A web-controlled TFT display project built on **ESP8266 + ST7735 + SD Card**.

This project allows you to upload and display images on a TFT screen directly via a web browser over WiFi — no firmware reflash required, no PC software needed.

---

## 🚀 Features

* 📡 WiFi STA mode with automatic AP provisioning
* 🌐 Built-in Web Server (ESP8266WebServer)
* 📂 Web-based SD File Browser
* 🖼 Display images on TFT via HTTP API
* 📁 Structured SD card folders
* 🎯 Automatic centering of 128×128 images on 160×128 display
* 🧩 Modular architecture
* 🤖 Developed using **Human × AI Engineering**

---

## 🖥 How It Works

1. Power on the device.
2. Connect to WiFi (automatic provisioning if needed).
3. Open the web interface:

   ```
   http://DEVICE_IP/files
   ```
4. Select an image from `/roadsigns`.
5. Press **SHOW**.
6. The image is instantly rendered on the TFT display.

---

## 🔌 Hardware Used

* ESP8266
* TFT ST7735 (160×128)
* SD Card module
* SPI interface

---

## 📂 Project Structure

```
ESP8266_TFT_STA_Web_Server_WiFiSetup_SD.ino
wifi_provision.h
app_routes.h
sd_browser.h
img_draw.h
sd_test.h
```

---

### 📌 ESP8266_TFT_STA_Web_Server_WiFiSetup_SD.ino

Main project file.

Responsible for:

* SPI initialization
* TFT initialization
* SD card setup
* WiFi provisioning
* Web server startup
* Route registration
* Main loop execution

---

### 📌 wifi_provision.h

WiFi configuration module.

* First-time AP mode
* Web form for SSID/password
* Credential storage
* Automatic switch to STA mode
* WiFi reset button support

---

### 📌 app_routes.h

Core web routes and application logic.

Handles:

* Main control pages
* Application API endpoints

---

### 📌 sd_browser.h

Web-based SD file manager.

Provides:

* `/files` — file browser interface
* `/api/list?dir=/...` — JSON directory listing
* `/sd?path=/...` — stream file to browser
* `/api/show?file=/...` — render image on TFT

---

### 📌 img_draw.h

TFT image rendering module.

Supports:

* 16-bit RGB565 BMP
* 24-bit BMP
* Centered 128×128 rendering
* Full 160×128 canvas display

---

### 📌 sd_test.h

SD card initialization and diagnostics module.

---

## 📁 SD Card Structure

```
/apriltag
/qr
/roadsigns
/roadsigns_test
/config
```

### /roadsigns

Main image directory (BMP files supported).

### /roadsigns_test

Testing directory for development.

### /qr

QR code images.

### /apriltag

AprilTag marker images.

### /config

WiFi and system configuration files.

---

## 🔄 API Examples

### Display Image

```
/api/show?file=/roadsigns/znak-kirpich.bmp
```

### Display 160×128 Canvas Image

```
/api/show?file=/roadsigns/znak-kirpich_160x128_canvas.bmp&full=1
```

### List Directory

```
/api/list?dir=/roadsigns
```

---

## 🧠 Architecture Overview

The project is built with clear modular separation:

* Web logic separated from rendering logic
* SD file management isolated
* Image rendering encapsulated
* Easily portable to ESP32

---

## 🤝 Human × AI Engineering

This project was developed through a collaboration between human engineering design and AI-assisted code structuring and optimization.

AI does not replace the engineer — it enhances productivity and accelerates development.

---

## 🔮 Future Improvements

* Web-based file upload
* JPEG support
* Slideshow mode
* Caching improvements
* ESP32 version

---

## ⭐ Support the Project

If you find this project useful, please consider giving it a star on GitHub.

---
