# Guardian RX – Smart Medicine Reminder & Monitoring System

> ⚠️ **Disclaimer**
> Guardian RX is a DIY/educational IoT project. It is **not a certified medical device**.
> Use it for prototyping, learning, and demonstrations only. For actual patient care, always follow the guidance of licensed healthcare professionals.

---

## 📌 Project Overview

Guardian RX is an IoT-based medicine reminder system built around two devices:

- **Pillbox (ROBO ESP32)** – monitors pill container lids, displays schedules, sends alerts.  
- **Wristband (ESP32-C3)** – receives reminder signals via Wi-Fi and vibrates to notify the patient.


---

##  Features
- 3 × IR sensors detect when each container is opened.  
- LCD shows container info + countdown timer.  
- Buzzer and Telegram alert when time is up.  
- Wristband vibrates wirelessly via Wi-Fi HTTP trigger.  
- Single Li-Po battery for both ESP32-C3 + Maker Drive.  

---

##  Hardware

##  Estimated Cost (Prototype, MYR)

| Item                       | Qty | Unit (RM) | Subtotal (RM) | Link                                                            |
| -------------------------- | --: | --------: | ------------: | --------------------------------------------------------------- |
| ROBOESP 32                 |   1 |     59.60 |         59.60 | [Cytron](https://my.cytron.io/p-robo-esp32)                     |
| ESP 32 Board               |   1 |     29.00 |         29.00 | [Cytron](https://my.cytron.io/p-robo-esp32)                     |
| IR Sensor Module           |   3 |      1.90 |          7.60 | [Cytron](https://my.cytron.io/p-infrared-sensor-module)         |
| Grove LCD Display          |   1 |        34 |            34 | [Cytron](https://my.cytron.io/p-grove-16-x-2-lcd-white-on-blue) |
| Grove Button               |   3 |      9.45 |          37.8 | [Cytron](https://my.cytron.io/p-grove-button)                   |
| Seeed XIAO ESP32C3         |   1 |     33.50 |         33.50 | [Cytron](https://my.cytron.io/p-seeed-xiao-esp32c3)             |
| MAKER DRIVE                |   1 |     24.70 |         24.70 | [TinyURL](https://tinyurl.com/DRV2605L-haptic)                  |
| Vibrating Motor            |   1 |      4.50 |          4.50 | [Cytron](https://my.cytron.io/p-mini-disc-vibrating-motor-1027) |
| LiPo Battery 3.7 V 800 mAh |   1 |        13 |            13 | [TinyURL](https://tinyurl.com/lipo-battery-v)                   |
| Pillbox                    |   2 |      3.55 |          7.10 | [TinyURL](https://tinyurl.com/pillbox-fyp)                      |
| 3D Print Enclosure         |   1 |    est 30 |            30 | —                                                               |

**Total (approx.) = RM 277.30** + Forgot to add ESP32 board
---

## 🔌 Wiring Overview

### Pillbox (ROBO ESP32)
| Function | Pin | Notes |
|-----------|-----|-------|
| IR Sensor 1 | GPIO 16 | Container 1 |
| IR Sensor 2 | GPIO 25 | Container 2 |
| IR Sensor 3 | GPIO 26 | Container 3 |
| LCD I2C | GPIO 21 (SDA), 22 (SCL) | Grove LCD |
| Buttons | 33 (SET), 32 (UP), 39 (DOWN) | Pull-ups enabled |
| Buzzer | GPIO 23 | Onboard |

### Wristband (ESP32-C3)
| Function | Pin | Notes |
|-----------|-----|-------|
| Maker Drive PWM Input | D10 | Control vibration motor |
| Maker Drive VCC | 3V3 or VBAT | From battery |
| Maker Drive GND | GND | Common ground |
| Li-Po + → Switch → BAT+ | Battery positive line |
| Li-Po – → BAT– | Battery negative line |

---

##  Firmware

## Software Setup
install CH210X driver if first time use ESP32 : https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers.zip

### 1. Install Arduino IDE
1. Install latest **Arduino IDE 2.x**  
2. Add ESP32 board support "File -> Preferences -> Additional board manager URLs :  https://espressif.github.io/arduino-esp32/package_esp32_index.json
3. Install **“ESP32 by Espressif Systems”** from Boards Manager.  
4. Select board:
- Pillbox → **ESP32 Dev Module**  
- Wristband → **Seeed XIAO ESP32-C3**

---

### 2. Create Telegram Bot
1. Open Telegram and talk to **@BotFather**.  
2. Send `/newbot` and follow prompts.  
3. Copy your **bot token** (e.g. `123456789:ABC-XYZ...`).  
4. Get your **chat ID**:  
- Start your bot, send a message.  
- Visit:  
  `https://api.telegram.org/bot<your_token>/getUpdates`  
- Note `"chat":{"id": ... }`.

Put both into `config.h`:

```cpp
#define TELEGRAM_BOT_TOKEN "123456789:ABC-XYZ..."
#define TELEGRAM_CHAT_ID   "123456789"
```
---

### 3.Configure Wi-Fi

Edit in config.h:
#define WIFI_SSID   "YourWiFiName"
#define WIFI_PASS   "YourPassword"

---

### 4. Upload Firmware
Upload guardianrx.ino to the ROBO ESP32.
Upload wristband.ino to the XIAO ESP32-C3.
Check Serial Monitor for Wi-Fi IP addresses.

##  How to use?
1. Power both boards (battery or USB).
2. On LCD:
- Use UP/DOWN to choose container.
- Press SET to edit hours → SET → edit minutes → SET to start.
3. Countdown runs automatically.
4. When time = 0:
- Buzzer beeps.
- Telegram sends “Take medicine from Container X”.
- Wristband vibrates once.
5. Open the pillbox → IR sensor logs to Telegram “Container X opened”

## MIT License

### Copyright (c) 2025 yoshiosenpai
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.