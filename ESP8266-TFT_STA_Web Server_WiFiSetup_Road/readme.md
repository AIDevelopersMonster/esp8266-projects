

````markdown
# 🚦 ESP8266 TFT Traffic Control Panel (Wi-Fi + Web GUI)

A smart IoT project using **ESP8266 + TFT ST7735 display**.

This device connects to your home Wi-Fi router and provides a **Web Control Panel**  
to display traffic-style road signs on a TFT screen.

Control everything directly from your **phone or computer browser**.

---

## 🚀 Features

- 📡 ESP8266 connects to router (STA mode)
- 🌐 Wi-Fi setup through browser (SSID + Password input)
- 📱 Works on mobile and PC
- 🎨 TFT graphical road sign display
- 🛑 Traffic control icons:

  - ⬅ Left arrow  
  - ➡ Right arrow  
  - ↩ U-turn / Back  
  - 🟢 Allowed movement  
  - ❌ Stop / Forbidden  

- 🧹 Clear screen button
- Instant updates in real time

---

## 🎬 Video Demo

<p align="center">
  <a href="https://youtu.be/85W4QjkzMdM">
    <img src="https://img.youtube.com/vi/85W4QjkzMdM/maxresdefault.jpg" width="800">
  </a>
</p>

🔗 Watch on YouTube:  
https://youtu.be/85W4QjkzMdM

---

# 🧰 Hardware Required

- ESP8266 NodeMCU (ESP-12E)
- TFT Display 1.8" SPI (ST7735)
- USB cable (for flashing)

---

# 🔌 Wiring

## SPI Pins (Required)

| TFT Pin | ESP8266 |
|--------|---------|
| SCK    | D5 (GPIO14) |
| MOSI   | D7 (GPIO13) |
| MISO   | D6 (only if SD module is used) |

## Control Pins

| TFT Pin | ESP8266 |
|--------|---------|
| CS     | D2 (GPIO4) |
| DC     | D1 (GPIO5) |
| RST    | Board RESET pin (recommended) |
| VCC    | 3.3V |
| GND    | GND |

---

# 🛠 Installation (Arduino IDE)

## 1️⃣ Install Arduino IDE  
https://www.arduino.cc/

---

## 2️⃣ Install ESP8266 Board Support

Add this URL in:

**File → Preferences → Additional Boards Manager URLs**

```text
http://arduino.esp8266.com/stable/package_esp8266com_index.json
````

Then install **ESP8266** in Boards Manager.

---

## 3️⃣ Install Required Libraries

In Library Manager install:

* Adafruit GFX Library
* Adafruit ST7735 and ST7789 Library

---

## 4️⃣ Upload Sketch

Open the project sketch:

```text
esp8266_tft_traffic_web.ino
```

Select board:

```text
NodeMCU 1.0 (ESP-12E Module)
```

Upload speed:

```text
115200
```

Click **Upload** 🚀

---

# 🌐 How It Works

### First Start (Wi-Fi Setup)

1. ESP8266 creates a setup Wi-Fi network:

```text
ESP-Setup
```

2. Connect with phone or PC

3. Open in browser:

```text
http://192.168.4.1
```

4. Enter your router SSID + password

5. ESP restarts and connects automatically

---

### Control Mode (Router Network)

After reboot, Serial Monitor will show device IP:

```text
IP: 192.168.1.68
```

Open in browser:

```text
http://192.168.1.68
```

You will see the Traffic Control Panel.

---

# 🎮 Web Control Buttons

* ⬅ Left arrow sign
* ➡ Right arrow sign
* ↩ Back / U-turn sign
* 🟢 GO (green arrow)
* ❌ STOP (red cross)
* Clear Screen

Updates appear instantly on the TFT display.

---

# ⚠ Troubleshooting

## White Screen on TFT

Try changing initialization:

```cpp
tft.initR(INITR_BLACKTAB);
```

To:

```cpp
INITR_GREENTAB
INITR_REDTAB
```

Different TFT modules require different configs.

---

## ESP-Setup Network Not Visible

* Reset EEPROM or press BOOT reset (if implemented)
* Ensure ESP8266 is powered correctly (3.3V)

---

# 📈 Future Improvements

* Real road-sign style arrows (ГОСТ design)
* WebSocket instant control
* OTA firmware updates
* Home Assistant integration
* Animated arrows and timers

---

# 📜 License

MIT License
Free to use, modify and share.

---

## ⭐ Support

If you like this project — give it a star ⭐ on GitHub!

```

---

