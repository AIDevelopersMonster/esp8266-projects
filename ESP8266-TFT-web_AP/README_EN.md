---

# ESP8266 TFT Control (Wi-Fi AP + Web Interface)

Control a **1.8" TFT display (ST7735)** over Wi-Fi using an **ESP8266 Web Interface**.

The ESP8266 creates its own Wi-Fi network (Access Point), and you can control the screen directly from any browser — phone or PC.

---

## 🚀 Features

* 📡 ESP8266 works as a Wi-Fi Access Point
* 🌐 Built-in Web Control Panel
* 🎨 Background color picker
* ✏ Print text on the TFT screen
* 📍 Adjustable X/Y coordinates
* 🔠 Font size control
* 📱 Works perfectly on mobile
* 🔌 No USB needed after flashing

---

## 🎬 Project Video (AP + Web Version)

<p align="center">
  <a href="https://youtu.be/3CkwzwPAMKk">
    <img src="https://img.youtube.com/vi/3CkwzwPAMKk/maxresdefault.jpg" width="800">
  </a>
</p>

📺 This video demonstrates:

* ESP8266 running in Access Point mode
* TFT control through a browser
* Live text + color updates
* Full Web GUI without USB

🔗 Watch on YouTube:
[https://youtu.be/3CkwzwPAMKk](https://youtu.be/3CkwzwPAMKk)

---

# 🧰 Requirements

## Hardware

* ESP8266 (NodeMCU / Wemos D1 mini)
* 1.8" SPI TFT Display (ST7735)
* USB cable (for flashing)

---

# 🔌 Wiring

## SPI Pins (Required)

| TFT Pin | ESP8266 Pin             |
| ------- | ----------------------- |
| SCK     | D5 (GPIO14)             |
| MOSI    | D7 (GPIO13)             |
| MISO    | D6 (only if SD is used) |

## Control Pins

| TFT Pin | ESP8266 Pin                   |
| ------- | ----------------------------- |
| CS      | D2 (GPIO4)                    |
| DC      | D1 (GPIO5)                    |
| RST     | Board RESET pin (recommended) |
| VCC     | 3.3V                          |
| GND     | GND                           |

---

# 🛠 Installation (Arduino IDE)

## 1️⃣ Install Arduino IDE

[https://www.arduino.cc/](https://www.arduino.cc/)

---

## 2️⃣ Install ESP8266 Board Support

Go to:

**File → Preferences → Additional Boards Manager URLs**

Add:

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

Then:

**Tools → Boards Manager → Install ESP8266**

---

## 3️⃣ Install Required Libraries

Open:

**Sketch → Include Library → Manage Libraries**

Install:

* Adafruit GFX Library
* Adafruit ST7735 and ST7789 Library

---

## 4️⃣ Upload the Sketch

Open:

```
esp8266_tft_web.ino
```

Select board:

```
NodeMCU 1.0 (ESP-12E Module)
```

Baud rate:

```
115200
```

Click **Upload** 🚀

---

# 📡 Usage

After flashing, ESP8266 creates a Wi-Fi network:

```
ESP8266-TFT
```

Password:

```
12345678
```

Connect to the network.

Open in your browser:

```
http://192.168.4.1
```

A Web GUI will appear, allowing full TFT control.

---

# 🌐 Web Interface Functions

### Text Display

* Enter text
* Set X/Y position
* Choose font size
* Pick text color

### Background Control

* Pick background color
* Clear screen instantly

All changes update live.

---

# ⚠ Troubleshooting

## White Screen Issue

Try changing initialization:

```cpp
tft.initR(INITR_BLACKTAB);
```

To:

```cpp
INITR_GREENTAB
INITR_REDTAB
```

Different TFT modules require different tabs.

---

## Wi-Fi Not Showing Up

* Password must be at least 8 characters
* Ensure AP mode is enabled:

```cpp
WiFi.mode(WIFI_AP);
```

---

# 📈 Future Improvements

* Station Mode (connect to home Wi-Fi)
* WebSocket real-time control
* TFT preview inside browser
* BMP image loading from SD card
* OTA firmware updates
* Full menu UI system

---

# 📜 License

MIT License
Free to use, modify, and share.

---

