#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "rgb_lcd.h"
#include "config.h"

// ================= LCD =================
rgb_lcd lcd;

// ================= Config =================
#define NUM_CONTAINERS 3
int irPins[NUM_CONTAINERS] = {PIN_IR1, PIN_IR2, PIN_IR3};

struct Container {
  uint16_t hh=0, mm=0;   // user-set duration
  long remaining=0;      // countdown seconds
  bool active=false;
};
Container containers[NUM_CONTAINERS];
int currentContainer = 0;  // selected container index

// ================= Button Debounce =================
bool readButton(int pin) {
  static uint32_t lastMs[50] = {0};
  static bool lastState[50] = {false};
  bool now = (digitalRead(pin)==LOW);
  if (now != lastState[pin] && millis()-lastMs[pin] > 150) {
    lastMs[pin] = millis();
    lastState[pin] = now;
    return now; // true on new press
  }
  return false;
}

// ================= Buzzer =================
void buzz(uint16_t ms=500){
  digitalWrite(PIN_BUZZ,HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZ,LOW);
}

// ================= Telegram =================
void sendTelegram(const String &msg) {
  if(WiFi.status()!=WL_CONNECTED) return;
  HTTPClient http;
  String url="https://api.telegram.org/bot"+String(TELEGRAM_BOT_TOKEN)+
             "/sendMessage?chat_id="+TELEGRAM_CHAT_ID+
             "&text="+msg;
  http.begin(url); 
  int code=http.GET();
  Serial.printf("[Telegram] %s | Code:%d\n", msg.c_str(), code);
  http.end();
}

// ================= Tasks =================

// --- IR Monitoring ---
void irTask(void *pv) {
  bool lastState[NUM_CONTAINERS] = {HIGH,HIGH,HIGH};
  while(1){
    for(int i=0;i<NUM_CONTAINERS;i++){
      int val = digitalRead(irPins[i]);
      if(val==LOW && lastState[i]==HIGH){ // falling edge
        String msg = "Container " + String(i+1) + " opened";
        sendTelegram(msg);
      }
      lastState[i]=val;
    }
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

// --- Countdown Task ---
void countdownTask(void *pv){
  while(1){
    for(int i=0;i<NUM_CONTAINERS;i++){
      if(containers[i].active && containers[i].remaining>0){
        containers[i].remaining--;
        if(containers[i].remaining==0){
          buzz(700);
          String msg="Reminder: Timer finished for Container "+String(i+1);
          sendTelegram(msg);
          containers[i].active=false; // stop timer
        }
      }
    }
    vTaskDelay(1000/portTICK_PERIOD_MS); // tick every second
  }
}

// --- UI Task ---
enum UiMode { VIEW, SET_HH, SET_MM };
UiMode uiMode = VIEW;
uint32_t lastLcdUpdate=0;

void uiTask(void *pv){
  lcd.begin(16,2);
  lcd.setRGB(0,255,255);

  while(1){
    if (millis()-lastLcdUpdate > 500) {
      lastLcdUpdate=millis();
      lcd.clear();

      if(uiMode==VIEW){
        lcd.setCursor(0,0);
        lcd.print("Container "); lcd.print(currentContainer+1);

        lcd.setCursor(0,1);
        if(containers[currentContainer].active && containers[currentContainer].remaining>0){
          int h=containers[currentContainer].remaining/3600;
          int m=(containers[currentContainer].remaining%3600)/60;
          int s=containers[currentContainer].remaining%60;
          char buf[16]; sprintf(buf,"%02d:%02d:%02d",h,m,s);
          lcd.print(buf);
        } else {
          lcd.print("Timer not set");
        }
      }
      else { // SET_HH / SET_MM
        lcd.setCursor(0,0); 
        lcd.print("Set C"); lcd.print(currentContainer+1);
        char buf[8]; sprintf(buf,"%02d:%02d",containers[currentContainer].hh,containers[currentContainer].mm);
        lcd.setCursor(0,1); lcd.print(buf);
        // cursor indicator
        if(uiMode==SET_HH) lcd.setCursor(0,1); 
        else lcd.setCursor(3,1);
        lcd.blink(); 
      }
    }

    // ===== Button Handling =====
    if(readButton(PIN_BTN_UP)){
      if(uiMode==VIEW){
        currentContainer=(currentContainer+1)%NUM_CONTAINERS;
      } else if(uiMode==SET_HH){
        containers[currentContainer].hh=(containers[currentContainer].hh+1)%24;
      } else if(uiMode==SET_MM){
        containers[currentContainer].mm=(containers[currentContainer].mm+1)%60;
      }
    }

    if(readButton(PIN_BTN_DOWN)){
      if(uiMode==VIEW){
        currentContainer=(currentContainer+NUM_CONTAINERS-1)%NUM_CONTAINERS;
      } else if(uiMode==SET_HH){
        containers[currentContainer].hh=(containers[currentContainer].hh+23)%24;
      } else if(uiMode==SET_MM){
        containers[currentContainer].mm=(containers[currentContainer].mm+59)%60;
      }
    }

    if(readButton(PIN_BTN_SET)){
      if(uiMode==VIEW){
        uiMode=SET_HH; // enter edit mode
      } else if(uiMode==SET_HH){
        uiMode=SET_MM; // switch to minute
      } else if(uiMode==SET_MM){
        // Save timer as countdown
        containers[currentContainer].remaining = containers[currentContainer].hh*3600 + containers[currentContainer].mm*60;
        if(containers[currentContainer].remaining>0) containers[currentContainer].active=true;
        lcd.clear(); lcd.print("Saved!"); 
        vTaskDelay(1000/portTICK_PERIOD_MS);
        uiMode=VIEW; // back to view
        lcd.noBlink();
      }
    }

    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}

// ================= Setup =================
void setup(){
  Serial.begin(115200);

  // Pins
  pinMode(PIN_BTN_SET,INPUT_PULLUP);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN,INPUT_PULLUP);
  pinMode(PIN_BUZZ,OUTPUT);
  for(int i=0;i<NUM_CONTAINERS;i++) pinMode(irPins[i],INPUT);

  // WiFi
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] Connected! IP="+WiFi.localIP().toString());

  // Tasks
  xTaskCreatePinnedToCore(irTask,"IR",4096,NULL,1,NULL,1);
  xTaskCreatePinnedToCore(uiTask,"UI",4096,NULL,1,NULL,1);
  xTaskCreatePinnedToCore(countdownTask,"Countdown",4096,NULL,1,NULL,1);
}

void loop(){}
