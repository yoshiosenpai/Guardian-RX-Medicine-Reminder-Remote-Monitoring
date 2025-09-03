# Guardian RX – Smart Medicine Reminder & Monitoring System

> ⚠️ **Disclaimer**
> Guardian RX is a DIY/educational IoT project. It is **not a certified medical device**.
> Use it for prototyping, learning, and demonstrations only. For actual patient care, always follow the guidance of licensed healthcare professionals.

---

## 📌 Project Overview

**Guardian RX** is a smart pillbox system with an optional **BLE wristband**.

* Helps patients remember medication with **local display, buzzer, and reminders**.
* Logs adherence remotely via **Node-RED + MQTT**.
* Alerts caregivers with **Telegram/Email notifications**.
* Extends to a **wearable wristband** that vibrates when alarms trigger.

---

## 📂 Repository Structure

```
guardian-rx/
├─ firmware/
│  ├─ ArduinoIDE/GuardianRX/     # Pillbox firmware
│  ├─ ArduinoIDE/Wristband/      # Wristband firmware
├─ backend/nodered/              # Node-RED flow + webhook
├─ hardware/                     # Wiring tables, BOM
├─ data/                         # Example CSV/Sheet schema
├─ safety/                       # Disclaimer
└─ README.md
```
---

##  Features

### Pillbox (ROBO ESP32)

1. **IR Sensor Monitoring** – Detects pillbox interaction per container.
2. **LCD Display (I²C)** – Shows medicine name + dosage instructions.
3. **Buttons (SET / UP / DOWN)** – Configure reminders locally.
4. **Built-in Buzzer** – Audio alarm at reminder time.
5. **MQTT Publish** – Events logged to Node-RED → CSV + Google Sheets.
6. **Telegram/Email Alerts** – Caregivers notified on *LATE* or *MISSED*.

### Wristband (XIAO ESP32C3)

1. **BLE Peripheral** – Pairs with the pillbox as BLE Central.
2. **DRV2605L Haptic Driver + ERM motor** – Provides vibration feedback.
3. **Custom GATT service** – Receives simple `[pattern, repeats, strength]` packets.
4. **Battery-powered** – Runs on 3.7 V LiPo (charged via USB-C or JST-PH pads).
5. **Wearable** – 3D-printed capsule + wrist strap for patient comfort.

---

##  Hardware

### Pillbox Unit

* ROBO ESP32 (main controller, built-in buzzer, Grove ports)
* 4 × IR Sensor Modules (one per container)
* Grove LCD Display (16×2 or 20×4, I²C)
* 3 × Grove Buttons (SET, UP, DOWN)
* Pillbox enclosure (off-the-shelf or 3D-printed)
* Grove cables

### Wristband Unit

* Seeed XIAO ESP32C3 (with LiPo pads)
* DRV2605L haptic driver (I²C)
* Vibrating ERM motor (3 V)
* LiPo 3.7 V 800 mAh + JST-PH 2.0 female connector
* 3D-printed capsule enclosure
* Wrist strap (Velcro/TPU)

> 🔧 Note: The XIAO ESP32C3 version in the BOM requires you to **solder a JST-PH connector** or LiPo wires to the `BAT+/BAT-` pads. It already has a charging circuit onboard.

##  Estimated Cost (Prototype, MYR)

| Item                       | Qty | Unit (RM) | Subtotal (RM) | Link                                                            |
| -------------------------- | --: | --------: | ------------: | --------------------------------------------------------------- |
| ROBOESP 32                 |   1 |     59.60 |         59.60 | [Cytron](https://my.cytron.io/p-robo-esp32)                     |
| IR Sensor Module           |   4 |      1.90 |          7.60 | [Cytron](https://my.cytron.io/p-infrared-sensor-module)         |
| Grove LCD Display          |   1 |        34 |            34 | [Cytron](https://my.cytron.io/p-grove-16-x-2-lcd-white-on-blue) |
| Grove Button               |   4 |      9.45 |          37.8 | [Cytron](https://my.cytron.io/p-grove-button)                   |
| Seeed XIAO ESP32C3         |   1 |     33.50 |         33.50 | [Cytron](https://my.cytron.io/p-seeed-xiao-esp32c3)             |
| DRV2605L Haptic            |   1 |     24.70 |         24.70 | [TinyURL](https://tinyurl.com/DRV2605L-haptic)                  |
| Vibrating Motor            |   1 |      4.50 |          4.50 | [Cytron](https://my.cytron.io/p-mini-disc-vibrating-motor-1027) |
| LiPo Battery 3.7 V 800 mAh |   1 |        13 |            13 | [TinyURL](https://tinyurl.com/lipo-battery-v)                   |
| PH2.0 Female Connector     |   1 |         3 |             3 | [TinyURL](https://tinyurl.com/jht-connector)                    |
| Pillbox                    |   2 |      3.55 |          7.10 | [TinyURL](https://tinyurl.com/pillbox-fyp)                      |
| 3D Print Enclosure         |   1 |    est 30 |            30 | —                                                               |

**Total (approx.) = RM 277.30**

---

##  Wiring Overview

### Pillbox (ROBO ESP32 build)

| Function           | Module                  | Grove Port |       ESP32 Pin | Notes                                     |
| ------------------ | ----------------------- | ---------- | --------------: | ----------------------------------------- |
| LCD (I²C)          | Grove I²C LCD / RGB LCD | I²C port   | SDA=21 / SCL=22 | 5 V/3.3 V                                 |
| IR Sensor (C1..C4) | Grove IR Reflective     | D2–D5      | e.g., 16,17,4,5 | Digital input                             |
| Button – SET       | Grove Button            | D3         |    e.g., GPIO17 | INPUT\_PULLUP                             |
| Button – UP        | Grove Button            | D4         |     e.g., GPIO4 | INPUT\_PULLUP                             |
| Button – DOWN      | Grove Button            | D5         |     e.g., GPIO5 | INPUT\_PULLUP                             |
| Buzzer             | Built-in                | —          |    e.g., GPIO15 | Confirm actual pin from ROBO ESP32 pinout |

---

### Wristband (XIAO ESP32C3)

| Function         | Module                       | Pin           | Notes                                 |
| ---------------- | ---------------------------- | ------------- | ------------------------------------- |
| DRV2605L SDA/SCL | I²C bus                      | GPIO4 / GPIO5 | Check XIAO pinout                     |
| DRV2605L VCC/GND | 3V3 / GND                    | —             | 3.3 V                                 |
| Vibrating Motor  | ERM via DRV2605L OUT+ / OUT− | —             | Driven by DRV2605L                    |
| LiPo battery     | JST-PH → BAT+/BAT−           | Pads          | Board auto-charges when USB-C plugged |

---

##  Firmware

### Pillbox (ROBO ESP32)

* Arduino IDE
* Libraries: `LiquidCrystal_I2C` or Grove RGB LCD lib, `PubSubClient`, `ArduinoJson`, `Preferences`, `WiFi`, `NimBLE-Arduino`
* Modes:

  * **Scheduler**: compares current time vs reminders
  * **Alerts**: buzzer + LCD update + MQTT event
  * **BLE Central**: on alarm, connect to wristband → send `[pattern,repeats,strength]`
* MQTT events published to `guardianrx/<deviceId>/med/1/event`

### Wristband (XIAO ESP32C3)

* Arduino IDE
* Libraries: `NimBLE-Arduino`, `Adafruit_DRV2605`, `Wire`
* BLE Peripheral: advertises custom service
* Handles write requests to vibrate motor
* Runs on LiPo battery, charges via USB-C

---

##  MQTT / BLE Protocol

**MQTT (Pillbox → Node-RED):**

```json
{
  "type": "taken_ack",
  "ts": 1735900200,
  "med": "Metformin 500mg",
  "dose": "1 tab after meal",
  "due": "08:00",
  "rssi": -61
}
```

**BLE Write (Pillbox → Wristband):**

```text
[pattern_id, repeats, strength]
```

* `1` = ALARM
* `2` = LATE
* `3` = MISSED
* `4` = SHORT

---

##  Backend (Node-RED)

* **MQTT In** → Normalize payload
* **CSV append** → `/data/guardianrx.csv`
* **Google Sheets webhook** → Append row
* **Telegram / Email Alerts** → Immediate on *LATE* or *MISSED*

See: `backend/nodered/flows.guardianrx.json`

---

##  Test Plan

* **Due Reminder** → LCD shows med info, buzzer + vibrate on wristband
* **IR Sensor** → triggers on interaction
* **SET Button** → marks `taken_ack` if within window
* **Late Window** → logs `late`, vibrates pattern 2
* **Missed** → logs `missed`, vibrates pattern 3
* **MQTT Offline** → still vibrates + buzzer locally; syncs later
* **Battery** → wristband runs off LiPo, charges via USB-C

---


