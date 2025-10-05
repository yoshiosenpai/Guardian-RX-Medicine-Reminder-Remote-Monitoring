#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "rgb_lcd.h"
#include "config.h"

#ifndef WRISTBAND_URL
#define WRISTBAND_URL "http://192.168.137.214/vibrate"   // <- set IP ESP32-C3 IP
#endif


rgb_lcd lcd;


#define NUM_CONTAINERS 3
int irPins[NUM_CONTAINERS] = { PIN_IR1, PIN_IR2, PIN_IR3 };

struct Container {
  uint16_t hh = 0;        
  uint16_t mm = 0;        
  long     remaining = 0; 
  bool     active = false;
};
Container containers[NUM_CONTAINERS];
int currentContainer = 0;  


static bool readButton(int pin) {
  static uint32_t lastMs[64]  = {0};
  static uint8_t  lastLvl[64] = {HIGH};
  uint8_t now = digitalRead(pin);
  if (now != lastLvl[pin] && (millis() - lastMs[pin] > 120)) {
    lastMs[pin]  = millis();
    lastLvl[pin] = now;
    return (now == LOW);            
  }
  return false;
}


static void buzz(uint16_t ms = 500) {
  pinMode(PIN_BUZZ, OUTPUT);
  digitalWrite(PIN_BUZZ, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZ, LOW);
}


static void telegramSend(const String &msg) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN)
             + "/sendMessage?chat_id=" + TELEGRAM_CHAT_ID
             + "&text=" + msg;
  http.begin(url);
  int code = http.GET();
  Serial.printf("[Telegram] %s | Code: %d\n", msg.c_str(), code);
  http.end();
}


static void notifyWristband() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(WRISTBAND_URL);
  int code = http.GET();
  Serial.printf("[Wristband] GET %s -> %d\n", WRISTBAND_URL, code);
  http.end();
}


static void irTask(void *pv) {
  bool last[NUM_CONTAINERS] = { HIGH, HIGH, HIGH };
  for (int i = 0; i < NUM_CONTAINERS; i++) pinMode(irPins[i], INPUT);

  while (true) {
    for (int i = 0; i < NUM_CONTAINERS; i++) {
      int v = digitalRead(irPins[i]);
      if (v == LOW && last[i] == HIGH) {
        telegramSend("Container " + String(i + 1) + " opened");
      }
      last[i] = v;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


static void countdownTask(void *pv) {
  while (true) {
    for (int i = 0; i < NUM_CONTAINERS; i++) {
      if (containers[i].active && containers[i].remaining > 0) {
        containers[i].remaining--;
        if (containers[i].remaining == 0) {
          buzz(700);
          telegramSend("Reminder: Take medicine from Container " + String(i + 1));
          notifyWristband();
          containers[i].active = false;  
        }
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  
  }
}


enum UiMode { VIEW, SET_HH, SET_MM };
static UiMode uiMode = VIEW;
static uint32_t lastLcd = 0;

static void uiTask(void *pv) {
  
  pinMode(PIN_BTN_SET,  INPUT_PULLUP);
  pinMode(PIN_BTN_UP,   INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 255);

  while (true) {

    if (millis() - lastLcd > 500) {
      lastLcd = millis();
      lcd.clear();

      if (uiMode == VIEW) {
        lcd.setCursor(0, 0);
        lcd.print("Container ");
        lcd.print(currentContainer + 1);

        lcd.setCursor(0, 1);
        if (containers[currentContainer].active && containers[currentContainer].remaining > 0) {
          long t = containers[currentContainer].remaining;
          int h = t / 3600;
          int m = (t % 3600) / 60;
          int s = t % 60;
          char buf[16]; snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
          lcd.print(buf);
        } else {
          lcd.print("Timer not set");
        }
        lcd.noBlink();
      } else {

        lcd.setCursor(0, 0);
        lcd.print("Set C");
        lcd.print(currentContainer + 1);
        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d",
                 containers[currentContainer].hh,
                 containers[currentContainer].mm);
        lcd.setCursor(0, 1);
        lcd.print(buf);
        lcd.blink();
        if (uiMode == SET_HH) lcd.setCursor(0, 1); else lcd.setCursor(3, 1);
      }
    }


    if (readButton(PIN_BTN_UP)) {
      if (uiMode == VIEW) {
        currentContainer = (currentContainer + 1) % NUM_CONTAINERS;
      } else if (uiMode == SET_HH) {
        containers[currentContainer].hh = (containers[currentContainer].hh + 1) % 24;
      } else {
        containers[currentContainer].mm = (containers[currentContainer].mm + 1) % 60;
      }
    }

    if (readButton(PIN_BTN_DOWN)) {
      if (uiMode == VIEW) {
        currentContainer = (currentContainer + NUM_CONTAINERS - 1) % NUM_CONTAINERS;
      } else if (uiMode == SET_HH) {
        containers[currentContainer].hh = (containers[currentContainer].hh + 23) % 24;
      } else {
        containers[currentContainer].mm = (containers[currentContainer].mm + 59) % 60;
      }
    }

    if (readButton(PIN_BTN_SET)) {
      if (uiMode == VIEW) {
        uiMode = SET_HH;   
      } else if (uiMode == SET_HH) {
        uiMode = SET_MM;   
      } else {
        
        containers[currentContainer].remaining =
            (long)containers[currentContainer].hh * 3600L +
            (long)containers[currentContainer].mm * 60L;
        containers[currentContainer].active = (containers[currentContainer].remaining > 0);

        lcd.clear(); lcd.print("Saved!"); delay(900);
        uiMode = VIEW; lcd.noBlink();
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}


void setup() {
  Serial.begin(115200);
  delay(300);

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WiFi] Connecting");
  uint8_t tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) { delay(250); Serial.print("."); tries++; }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Connected: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WiFi] Failed (continuing offline).");
  }

  
  xTaskCreatePinnedToCore(irTask,        "IR",        4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(countdownTask, "Countdown", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(uiTask,        "UI",        4096, NULL, 1, NULL, 1);
}

void loop() {

}
