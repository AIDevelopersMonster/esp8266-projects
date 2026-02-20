---

# ESP8266 TFT Control (Router / STA Mode)

Control a **1.8" TFT display (ST7735)** over your home Wi-Fi network using ESP8266 in **Station (STA) mode**.

The ESP8266 connects to your router and becomes a local network device.
You can control the TFT screen via browser using HTTP requests.

---

## 🚀 Features

* 📡 Connects to your home Wi-Fi (Router mode)
* 🌐 Built-in Web Server
* 📝 Display text via browser
* 🎨 Change background color
* 📍 Adjustable text position
* 🔠 Adjustable text size
* 🔄 Instant screen updates
* 🧹 Screen clearing support

---

## 🎬 Project Video (Router / Home Wi-Fi Version)

<p align="center">
  <a href="https://youtu.be/vjBUdD6EZ1c">
    <img src="https://img.youtube.com/vi/vjBUdD6EZ1c/maxresdefault.jpg" width="800">
  </a>
</p>

📺 In this video you will see:

- ESP8266 connected to a home router (STA mode)  
- TFT display control through local IP  
- Text and background updates via browser  
- HTTP API demo commands  
- Screen clearing examples  

🔗 Watch on YouTube:  
https://youtu.be/vjBUdD6EZ1c

---

# 🧰 Hardware Required

* ESP8266 (NodeMCU / Wemos D1 mini)
* 1.8" SPI TFT display (ST7735)
* USB cable (for flashing)

---

# 🔌 Wiring

## SPI Pins

| TFT Pin | ESP8266         |
| ------- | --------------- |
| SCK     | D5 (GPIO14)     |
| MOSI    | D7 (GPIO13)     |
| MISO    | D6 (if SD used) |

## Control Pins

| TFT Pin | ESP8266                   |
| ------- | ------------------------- |
| CS      | D2 (GPIO4)                |
| DC      | D1 (GPIO5)                |
| RST     | Board RESET (recommended) |
| VCC     | 3.3V                      |
| GND     | GND                       |

---

# 🛠 Installation

## 1️⃣ Install Arduino IDE

[https://www.arduino.cc/](https://www.arduino.cc/)

## 2️⃣ Install ESP8266 Board Support

Add to:

File → Preferences → Additional Boards Manager URLs

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

Install **ESP8266** from Boards Manager.

---

## 3️⃣ Install Required Libraries

* Adafruit GFX Library
* Adafruit ST7735 and ST7789 Library

---

## 4️⃣ Configure Wi-Fi Credentials

In the sketch:

```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

---

## 5️⃣ Upload the Sketch

Select:

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

After flashing, open **Serial Monitor**.

ESP8266 will display something like:

```
IP: 192.168.1.68
```

This is your device's local IP address.

---

# 🌐 HTTP API Commands

Replace `192.168.1.68` with your actual IP.

---

## 📝 Display Text

```
http://192.168.1.68/text?msg=Hello
```

With position and size:

```
http://192.168.1.68/text?msg=ESP8266&x=20&y=50&size=2
```

---

## 🎨 Change Background Color

Blue background:

```
http://192.168.1.68/fill?r=0&g=0&b=255
```

Red background:

```
http://192.168.1.68/fill?r=255&g=0&b=0
```

---

## 🧹 Clear Screen

Black background (clean screen):

```
http://192.168.1.68/fill?r=0&g=0&b=0
```

---

# 🔁 How It Works

ESP8266 runs:

* `WiFi.mode(WIFI_STA)`
* `ESP8266WebServer`
* REST-style HTTP API

It connects to your router and becomes accessible inside your local network.

---

# 🔥 AP vs Router Mode Comparison

| AP Mode                   | Router Mode                |
| ------------------------- | -------------------------- |
| ESP creates its own Wi-Fi | Connects to existing Wi-Fi |
| IP = 192.168.4.1          | IP assigned by router      |
| Standalone                | Full IoT integration       |

---

# 📈 Future Improvements

* Web GUI page (HTML interface)
* WebSocket real-time control
* Home Assistant integration
* Telegram bot control
* OTA firmware updates
* Image display from SD card

---

# 📜 License

MIT License
Free to use and modify.

---


