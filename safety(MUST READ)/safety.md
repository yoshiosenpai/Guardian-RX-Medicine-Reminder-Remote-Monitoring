## 🛡️ **Guardian RX Safety Guidelines**

> ⚠️ **Important:** The Guardian RX system uses Li-Po batteries, electronic sensors, and wireless communication modules.
> Follow these safety instructions to avoid damage, overheating, or injury.

---

### 🔋 **Battery Safety (Li-Po / Li-ion)**

1. **Polarity check:**
   * Red = `BAT+`, Black = `BAT–`.
   * Reversing polarity may permanently damage your board.

2. **Charging:**
   * The XIAO ESP32-C3 has a built-in charger (≈ 200 mA).
   * Only charge via USB-C; never connect an external charger in parallel.

3. **Never charge unattended.**
   * Place on a non-flammable surface (Li-Po bag or ceramic plate).

4. **Temperature limits:**
   * Stop operation if the battery becomes hot, swollen, or emits odor.

5. **Storage:**
   * If unused > 2 weeks, store at 3.7 – 3.8 V (~50 % charge).
   * Keep away from direct sunlight and metal objects.

---

### ⚡ **Electrical Safety**

1. **Disconnect power before wiring changes.**
2. **Never short-circuit the Li-Po battery.**
   * A direct short can cause fire or explosion.

3. **Secure all solder joints** with heat-shrink tubing or insulation tape.
4. **Ensure common ground** between all modules (ESP32, Maker Drive, sensors).
5. **Limit vibration motor current** to < 300 mA. The Maker Drive protects your GPIO, but avoid direct wiring from the pin to the motor.

---

### 🔌 **Power Switch & Charging**

* The **SPST rocker switch** should be placed **on the battery + line** only.

  ```
  Battery + → Switch → BAT+ (ESP32-C3)
  Battery – → BAT–
  ```
* Switching OFF cuts power to the system but **does not disable charging** — USB-C can still charge the battery safely.
* Do **not** toggle the switch rapidly while USB is connected.

---

### 🌡️ **Operational Safety**

1. Avoid placing the wristband motor directly on skin for long periods; continuous vibration can cause irritation.
2. Keep the IR sensors clean and dry for reliable detection.
3. Do not expose either board to water, high humidity, or dust.
4. Ensure good ventilation during charging.

---

### 🧑‍🔧 **Software Safety**

1. Always disconnect power before uploading new firmware.
2. Verify Wi-Fi credentials and Telegram token privately — never share them publicly.

---

### 🧯 **Emergency Handling**

If any of the following occurs:

* Battery swelling, smoke, or high temperature
* Burning smell or spark
* Rapid, uncontrolled vibration

→ Immediately **disconnect power**, move the device to a fire-safe area, and allow it to cool.
Dispose of damaged Li-Po cells properly at a recycling center.

---

### **Checklist Before Operation**

| Check               | Description                           | OK? |
| ------------------- | ------------------------------------- | --- |
| 🔋 Battery voltage  | Between 3.0 V – 4.2 V                 | ☐   |
| 🧵 Wiring polarity  | Red → BAT+, Black → BAT–              | ☐   |
| 🌐 Wi-Fi connected  | Confirm via Serial Monitor            | ☐   |
| 📱 Telegram working | Test “/start” message                 | ☐   |
| 📟 LCD display      | “Container 1 – Timer not set” visible | ☐   |
| 🔔 Buzzer & motor   | Function test via 1-minute timer      | ☐   |

---

### 📜 **Disclaimer**

This project is for educational and prototyping purposes.
I not responsible for damage or injury caused by improper assembly or unsafe handling.
Use common sense, test carefully, and never leave powered prototypes unattended.

---

