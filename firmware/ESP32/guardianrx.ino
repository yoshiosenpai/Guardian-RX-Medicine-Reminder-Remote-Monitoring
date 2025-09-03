/*
  Guardian RX — Pillbox (ROBO ESP32)
  Cody by Solehin Rizal
  Features:
    - LCD (I2C) shows med label/dosage + status
    - Buttons (SET/UP/DOWN) to set reminder HH:MM
    - IR sensor detects interaction
    - Buzzer alert on Due
    - BLE Central -> vibrate wristband (see ble_band.h)
    - MQTT publish events to Node-RED (CSV / Sheets / Alerts)
    - NVS stores config (label/dosage/time/enabled/windows)
*/

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"     // <-- fill WiFi/MQTT/TZ pins/flags there
#include "ble_band.h"   // <-- from previous message

// ======= Feature flags fallback (if not set in config.h) =======
#ifndef GDRX_ENABLE_MQTT
#define GDRX_ENABLE_MQTT 1
#endif
#ifndef GDRX_ENABLE_BLE
#define GDRX_ENABLE_BLE  1
#endif

// ======= I2C LCD =======
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR 0x27    // change if your LCD backpack is 0x3F
#endif
#ifndef LCD_COLS
#define LCD_COLS 16
#endif
#ifndef LCD_ROWS
#define LCD_ROWS 2
#endif
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// ======= Globals =======
Preferences kv;

enum Status : uint8_t { IDLE=0, DUE=1, TAKEN=2, LATE=3, MISSED=4 };

struct MedCfg {
  char label[48];
  char dosage[48];
  uint8_t hh;       // 24h hour
  uint8_t mm;       // minute
  bool enabled;
  uint16_t due_window_min;   // time to accept "Taken" as on-time
  uint16_t grace_min;        // after-window still counts as LATE
  uint16_t timeout_min;      // after this -> MISSED
};

struct State {
  Status status;
  time_t lastDueEpoch;       // epoch of today's due time
  time_t lastEventEpoch;
  bool alerting;
};
MedCfg cfg;
State st;

// ======= WiFi / MQTT =======
#if GDRX_ENABLE_MQTT
WiFiClient net;
PubSubClient mqtt(net);
char deviceId[24] = "rx-001";    // set a unique ID for each unit
#endif

// ======= Debounce helpers =======
struct Debounce {
  uint8_t pin;
  uint8_t stable;
  uint8_t last;
  uint32_t lastChange;
  uint16_t ms;
};
Debounce dbBtnSet { PIN_BTN_SET, HIGH, HIGH, 0, 120 };
Debounce dbBtnUp  { PIN_BTN_UP,  HIGH, HIGH, 0, 120 };
Debounce dbBtnDn  { PIN_BTN_DOWN,HIGH, HIGH, 0, 120 };
Debounce dbIR1    { PIN_IR1,     HIGH, HIGH, 0, 60  };

static inline uint8_t readDebounce(Debounce& d) {
  uint8_t now = digitalRead(d.pin);
  if (now != d.last) { d.lastChange = millis(); d.last = now; }
  if (millis() - d.lastChange > d.ms) d.stable = now;
  return d.stable;
}
static inline bool fellEdge(Debounce& d) {
  uint8_t before = d.stable;
  uint8_t now = readDebounce(d);
  return (before == HIGH && now == LOW);
}

// ======= Buzzer =======
static inline void buzzOnce(uint16_t onMs=100) {
  digitalWrite(PIN_BUZZ, HIGH); delay(onMs);
  digitalWrite(PIN_BUZZ, LOW);
}
static inline void buzzPatternDue() { buzzOnce(120); delay(120); buzzOnce(120); }
static inline void buzzPatternLate(){ buzzOnce(80);  delay(160); buzzOnce(80);  }
static inline void buzzPatternMiss(){ buzzOnce(180); delay(200); buzzOnce(180); }

// ======= Utilities =======
String fmtHM(uint8_t hh, uint8_t mm){ char b[6]; snprintf(b,sizeof(b), "%02u:%02u", hh, mm); return String(b); }

// ======= LCD helpers =======
void lcdShow(const String& l1, const String& l2) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(l1.substring(0, LCD_COLS));
  lcd.setCursor(0,1); lcd.print(l2.substring(0, LCD_COLS));
}

void drawStatus() {
  String s = "Idle";
  if (st.status==DUE) s="Due";
  else if (st.status==TAKEN) s="Taken";
  else if (st.status==LATE) s="Late";
  else if (st.status==MISSED) s="Missed";

  String l1 = "C1: " + String(cfg.label);
  String l2 = s + " @" + fmtHM(cfg.hh,cfg.mm);
  lcdShow(l1, l2);
}

// ======= Persist =======
void loadCfg(){
  kv.begin("gdrx", true);
  String l = kv.getString("label", "Metformin 500mg");
  String d = kv.getString("dosage","1 tab after meal");
  uint32_t hh = kv.getUInt("hh", 8), mm = kv.getUInt("mm", 0);
  bool en = kv.getBool("en", true);
  cfg.due_window_min = kv.getUInt("duewin", 10);
  cfg.grace_min      = kv.getUInt("grace", 20);
  cfg.timeout_min    = kv.getUInt("timeout", 30);
  kv.end();

  l.toCharArray(cfg.label, sizeof(cfg.label));
  d.toCharArray(cfg.dosage, sizeof(cfg.dosage));
  cfg.hh = (uint8_t)hh; cfg.mm = (uint8_t)mm; cfg.enabled = en;
}

void saveCfg(){
  kv.begin("gdrx", false);
  kv.putString("label", cfg.label);
  kv.putString("dosage", cfg.dosage);
  kv.putUInt("hh", cfg.hh); kv.putUInt("mm", cfg.mm);
  kv.putBool("en", cfg.enabled);
  kv.putUInt("duewin", cfg.due_window_min);
  kv.putUInt("grace", cfg.grace_min);
  kv.putUInt("timeout", cfg.timeout_min);
  kv.end();
}

void saveEvent(const char* evt){
  kv.begin("gdrx", false);
  kv.putString("lastevt", evt);
  kv.putULong64("lastts", (uint64_t)time(nullptr));
  kv.end();
}

// ======= Time =======
void connectWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0=millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-t0<15000) delay(250);
}

void syncTime(){
  configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2);
  uint32_t t0=millis();
  time_t now; tm info;
  do { time(&now); localtime_r(&now, &info); delay(200); }
  while(info.tm_year+1900 < 2024 && millis()-t0<10000);
}

bool isDueThisMinute(time_t now){
  tm lt; localtime_r(&now, &lt);
  return (lt.tm_hour==cfg.hh && lt.tm_min==cfg.mm);
}

time_t todayDueEpoch(time_t ref){
  tm lt; localtime_r(&ref, &lt);
  lt.tm_hour = cfg.hh; lt.tm_min = cfg.mm; lt.tm_sec = 0;
  return mktime(&lt);
}

// ======= MQTT =======
#if GDRX_ENABLE_MQTT
void mqttReconnect(){
  if (mqtt.connected()) return;
  while (WiFi.status()==WL_CONNECTED && !mqtt.connected()){
    if (mqtt.connect(deviceId)) break;
    delay(750);
  }
}

void publishEvent(const char* type) {
  if (!mqtt.connected()) mqttReconnect();
  StaticJsonDocument<256> d;
  d["type"] = type;
  d["ts"]   = (uint64_t)time(nullptr);
  d["med"]  = cfg.label;
  d["dose"] = cfg.dosage;
  d["due"]  = fmtHM(cfg.hh,cfg.mm);
  char buf[256]; size_t n = serializeJson(d, buf);

  String topic = String("guardianrx/") + deviceId + "/med/1/event";
  mqtt.publish(topic.c_str(), buf, n);
}
#endif

// ======= Status handling =======
void setStatus(Status s, const char* evt) {
  st.status = s;
  st.lastEventEpoch = time(nullptr);
  drawStatus();
  saveEvent(evt);

  // buzzer + BLE patterns
  switch (s) {
    case DUE:
      buzzPatternDue();
      #if GDRX_ENABLE_BLE
        gdrx_ble::vibrateAlarm();
      #endif
      break;
    case LATE:
      buzzPatternLate();
      #if GDRX_ENABLE_BLE
        gdrx_ble::vibrateLate();
      #endif
      break;
    case MISSED:
      buzzPatternMiss();
      #if GDRX_ENABLE_BLE
        gdrx_ble::vibrateMissed();
      #endif
      break;
    case TAKEN:
      buzzOnce(80);
      break;
    default: break;
  }

  #if GDRX_ENABLE_MQTT
    publishEvent(evt);
  #endif
}

// ======= UI: set time (long-press SET) =======
void uiSetTime(){
  // simple 2-step editor: hour then minute
  uint8_t hh = cfg.hh, mm = cfg.mm;
  uint8_t step = 0; // 0=hour,1=minute
  uint32_t blinkT=0; bool show=true;

  while (true){
    if (millis()-blinkT>350){ blinkT=millis(); show=!show; }
    String l1 = "Set Time:";
    String l2 = (step==0 && !show) ? String("--:") + (mm<10?"0":"")+String(mm)
                                   : (step==1 && !show) ? fmtHM(hh,0) + String(mm<10?"-":"") + "-"
                                   : fmtHM(hh,mm);
    lcdShow(l1, l2);

    // read buttons
    readDebounce(dbBtnUp); readDebounce(dbBtnDn); readDebounce(dbBtnSet);
    if (fellEdge(dbBtnUp)) {
      if (step==0) hh = (hh+1)%24;
      else mm = (mm+1)%60;
    }
    if (fellEdge(dbBtnDn)) {
      if (step==0) hh = (hh+23)%24;
      else mm = (mm+59)%60;
    }
    if (fellEdge(dbBtnSet)) {
      if (step==0) step=1;
      else { cfg.hh=hh; cfg.mm=mm; saveCfg(); drawStatus(); return; }
    }
    delay(10);
  }
}

// ======= Setup / Loop =======
void setup(){
  // pins
  pinMode(PIN_BTN_SET,   INPUT_PULLUP);
  pinMode(PIN_BTN_UP,    INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN,  INPUT_PULLUP);
  pinMode(PIN_IR1,       INPUT_PULLUP);   // adjust if sensor active-high
  pinMode(PIN_BUZZ,      OUTPUT);
  digitalWrite(PIN_BUZZ, LOW);

  // LCD
  Wire.begin();         // ROBO ESP32: SDA=21, SCL=22 by default
  lcd.init(); lcd.backlight();
  lcdShow("Guardian RX", "Booting...");

  // cfg
  loadCfg();
  drawStatus();

  // Wi-Fi / time
  connectWiFi();
  syncTime();

  // BLE
  #if GDRX_ENABLE_BLE
    gdrx_ble::init();
    gdrx_ble::findBand();   // optional pre-scan
  #endif

  // MQTT
  #if GDRX_ENABLE_MQTT
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqttReconnect();
  #endif

  st.status = IDLE;
  st.alerting = false;
  st.lastDueEpoch = todayDueEpoch(time(nullptr));
}

void loop(){
  time_t now = time(nullptr);

  // ---- Buttons (short press SET marks taken if due) ----
  bool setEdge = fellEdge(dbBtnSet);
  bool upEdge  = fellEdge(dbBtnUp);
  bool dnEdge  = fellEdge(dbBtnDn);

  // long-press SET (>1.2s) to edit time
  static uint32_t setPressT=0; static bool setHeld=false;
  uint8_t setLevel = readDebounce(dbBtnSet);
  if (setLevel==LOW && !setHeld){ setHeld=true; setPressT=millis(); }
  if (setLevel==HIGH && setHeld){
    uint32_t held = millis()-setPressT; setHeld=false;
    if (held > 1200){ uiSetTime(); }
  }

  // UP/DOWN do nothing outside editor; could add volume/brightness later
  (void)upEdge; (void)dnEdge;

  // ---- IR sensor (info only) ----
  static bool irWasLow=false;
  bool irLow = (readDebounce(dbIR1)==LOW); // adjust polarity if your sensor differs
  if (irLow && !irWasLow) {
    // log an interaction info event (optional)
    #if GDRX_ENABLE_MQTT
      publishEvent("ir_triggered");
    #endif
  }
  irWasLow = irLow;

  // ---- Scheduler ----
  // refresh today's due epoch if midnight passed
  if (now - st.lastDueEpoch > 24*3600) st.lastDueEpoch = todayDueEpoch(now);

  // trigger DUE at exact minute
  if (cfg.enabled && isDueThisMinute(now) && st.status!=DUE && st.status!=TAKEN) {
    st.lastDueEpoch = todayDueEpoch(now);
    st.alerting = true;
    setStatus(DUE, "reminder_fired");
  }

  // while DUE: beep pattern occasionally
  if (st.status==DUE && st.alerting) {
    static uint32_t lastBeep=0;
    if (millis()-lastBeep>2200){ buzzPatternDue(); lastBeep=millis(); }
  }

  // acknowledge TAKEN (SET short press) within windows
  if (st.status==DUE && setEdge) {
    time_t delta = now - st.lastDueEpoch;
    if (delta <= (cfg.due_window_min * 60UL)) {
      st.alerting=false; setStatus(TAKEN, "taken_ack");
    } else if (delta <= ((cfg.due_window_min + cfg.grace_min) * 60UL)) {
      st.alerting=false; setStatus(LATE, "taken_late");
    } else {
      // already beyond grace; ignore press or flag
    }
  }

  // timeout to MISSED
  if (st.status==DUE) {
    time_t delta = now - st.lastDueEpoch;
    if (delta > ((cfg.due_window_min + cfg.timeout_min) * 60UL)) {
      st.alerting=false; setStatus(MISSED, "timeout_missed");
    }
  }

  // show normal screen every few seconds
  static uint32_t lastDraw=0;
  if (millis()-lastDraw>4000){ drawStatus(); lastDraw=millis(); }

  // MQTT keepalive
  #if GDRX_ENABLE_MQTT
    if (WiFi.status()==WL_CONNECTED){
      if (!mqtt.connected()) mqttReconnect();
      mqtt.loop();
    }
  #endif

  delay(10);
}
