# Guardian RX — Wiring Table

##  Pillbox (ROBO ESP32 build)

| **Function**          | **Component / Module** | **ROBO ESP32 Pin**             | **Type**             | **Notes / Description**              |
| --------------------- | ---------------------- | ------------------------------ | -------------------- | ------------------------------------ |
| **IR Sensor 1**       | Infrared Module #1     | GPIO 16                        | Digital IN           | Detects pill removal for Container 1 |
| **IR Sensor 2**       | Infrared Module #2     | GPIO 25                        | Digital IN           | Detects pill removal for Container 2 |
| **IR Sensor 3**       | Infrared Module #3     | GPIO 26                        | Digital IN           | Detects pill removal for Container 3 |
| **LCD Display (I²C)** | Grove RGB 16×2 LCD     | SDA → GPIO 21<br>SCL → GPIO 22 | I²C Bus              | Shows container info & countdown     |
| **SET Button**        | Grove Button           | GPIO 33                        | Digital IN (Pull-Up) | Short press = select / enter menu    |
| **UP Button**         | Grove Button           | GPIO 32                        | Digital IN (Pull-Up) | Navigate containers / increment time |
| **DOWN Button**       | Grove Button           | GPIO 39 (Input Only)           | Digital IN           | Navigate containers / decrement time |
| **Buzzer**            | Onboard Buzzer         | GPIO 23                        | Digital OUT          | Beeps when timer ends                |
| **Wi-Fi**             | Built-in               | –                              | –                    | Used for Telegram & wristband HTTP   |
| **Power Input**       | USB 5 V / 3.7 V Li-Po  | 5 V or VBAT                    | Power                | Power source for main controller     |


## Wristband (XIAO ESP32C3 build)

| **Function**                | **Component / Module**               | **ESP32-C3 Pin**              | **Type**          | **Notes / Description**                      |
| --------------------------- | ------------------------------------ | ----------------------------- | ----------------- | -------------------------------------------- |
| **Vibration Motor Control** | Maker Drive PWM 1A → Vibration Motor | D10                           | Digital OUT (PWM) | Logic signal to Maker Drive for motor ON/OFF |
| **Maker Drive Power (VCC)** | Maker Drive                          | 3V3 or VBAT (~3.7 V)          | Power             | Use same battery as ESP32-C3                 |
| **Maker Drive GND**         | Maker Drive                          | GND                           | Ground            | Must share GND with ESP32-C3                 |
| **Li-Po Battery (+ve)**     | 3.7 V 1300 mAh Li-Po                 | → SPST Switch → BAT ( + ) Pad | Power             | Adds manual ON/OFF control                   |
| **Li-Po Battery (–ve)**     | Li-Po (Black Wire)                   | BAT ( – ) Pad                 | Power             | Common ground                                |
| **USB-C**                   | Built-in                             | –                             | Power + Data      | Charges Li-Po and uploads code               |
| **Wi-Fi Module**            | Built-in                             | –                             | –                 | Receives HTTP trigger from pillbox           |


---

## Quick Battery Flow Diagram (XIAO ESP32C3 build)
Li-Po (+)
   │
 [ SPST Switch ]
   │
   ├──> ESP32-C3  BAT(+)
   └──> Maker Drive VCC
Li-Po (–) ─────────────> GND (shared)
