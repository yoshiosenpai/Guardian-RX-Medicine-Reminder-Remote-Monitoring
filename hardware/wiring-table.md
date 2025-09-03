# Guardian RX — Wiring Table

> ⚠️ **note:** gpio numbers may vary slightly by ro\*\*bo esp32 revision. always check the **silkscreen labels** on your board and update both this file and `config.h`.

---

## 📦 Pillbox (ROBO ESP32 build)

| Function                | Module                                   | ROBO ESP32 Port | GPIO Example    | Notes                                                       |
| ----------------------- | ---------------------------------------- | --------------- | --------------- | ----------------------------------------------------------- |
| LCD (I²C)               | Grove 16×2/20×4 I²C LCD OR Grove RGB LCD | Grove I²C       | SDA=21 / SCL=22 | powered from 3.3 V / 5 V. typical I²C addr `0x27` / `0x3F`. |
| IR Sensor (Container 1) | Grove IR Reflective / Break-beam         | Grove D2        | GPIO16          | digital in. repeat for C2–C4 using other D ports.           |
| Button – SET            | Grove Button                             | Grove D3        | GPIO17          | input\_pullup, pressed = LOW.                               |
| Button – UP             | Grove Button                             | Grove D4        | GPIO4           | input\_pullup.                                              |
| Button – DOWN           | Grove Button                             | Grove D5        | GPIO5           | input\_pullup.                                              |
| Buzzer                  | Built-in (ROBO ESP32)                    | —               | GPIO15\*        | confirm exact buzzer pin from your board revision.          |
| Optional RGB LED        | Grove 3-ch LED module / discrete LEDs    | Grove D6–D8     | GPIO25/26/27    | add \~220 Ω resistors if using discrete LEDs.               |

---

## ⌚ Wristband (XIAO ESP32C3 build)

| Function     | Module / Part             | XIAO ESP32C3 Pin     | Notes                                                |
| ------------ | ------------------------- | -------------------- | ---------------------------------------------------- |
| DRV2605L SDA | Haptic driver I²C         | D4 (GPIO4)           | connect via I²C.                                     |
| DRV2605L SCL | Haptic driver I²C         | D5 (GPIO5)           | connect via I²C.                                     |
| DRV2605L VCC | 3.3 V                     | 3V3 pin              | power the haptic driver + motor.                     |
| DRV2605L GND | Ground                    | GND                  | common ground.                                       |
| ERM motor    | Vibrating coin motor      | DRV2605L OUT+ / OUT− | driven directly by driver.                           |
| LiPo battery | 3.7 V 800 mAh (protected) | BAT+ / BAT− pads     | **requires soldering** JST-PH 2.0 connector to pads. |
| USB-C        | Built-in port on XIAO     | —                    | used for charging & programming.                     |

---

## 🔧 Notes

* **IR sensors**: active-low; use debounce (\~60 ms) in firmware.
* **Buttons**: all use `INPUT_PULLUP`; press → LOW.
* **LCD**: if using Grove RGB LCD, use `Grove_LCD_RGB_Backlight` lib; if 16×2 backpack, use `LiquidCrystal_I2C` and scan for address.
* **Wristband power**:

  * XIAO ESP32C3 **does have a LiPo charger** onboard.
  * Solder JST-PH connector to `BAT+/BAT-` pads.
  * When USB-C is plugged, it will charge the LiPo automatically.

---

## ✅ Final pin map (example, update per silkscreen)

```text
Pillbox (ROBO ESP32)
  PIN_IR1      = GPIO16
  PIN_BTN_SET  = GPIO17
  PIN_BTN_UP   = GPIO4
  PIN_BTN_DOWN = GPIO5
  PIN_BUZZ     = GPIO15   // confirm buzzer pin
  I2C SDA/SCL  = 21 / 22

Wristband (XIAO ESP32C3)
  SDA          = GPIO4
  SCL          = GPIO5
  DRV2605L VCC = 3V3
  DRV2605L GND = GND
  LiPo         = BAT+/BAT- pads (via JST-PH connector)
```

---
