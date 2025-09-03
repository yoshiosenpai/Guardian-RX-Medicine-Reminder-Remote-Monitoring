/*
  Guardian RX — Wristband (BLE Peripheral + Haptics)\
  Code by Solehin Rizal
  Board: Seeed XIAO ESP32C3
  Haptic: DRV2605L (I2C)
  BLE: NimBLE (lightweight)

  GATT:
    Service UUID:       6E400001-B5A3-F393-E0A9-E50E24DCCA9E
    Characteristic UUID:6E400002-B5A3-F393-E0A9-E50E24DCCA9E  (Write Without Response)
    Payload (3 bytes): [pattern_id, repeats, strength]
      pattern_id: 1=ALARM, 2=LATE, 3=MISSED, 4=SHORT
      repeats:    1..5
      strength:   0..3 (higher = longer/stronger)

  Notes:
    - Solder JST-PH 2.0 to BAT+/BAT- pads (LiPo 3.7V).
    - The XIAO ESP32C3 variant exposes charging via USB-C.

*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include <NimBLEDevice.h>

// === BLE UUIDs (keep in sync with pillbox) ===
static const char* SVC_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
static const char* CHR_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

// === I2C pins for XIAO ESP32C3 (default) ===
#ifndef SDA
#define SDA 4
#endif
#ifndef SCL
#define SCL 5
#endif

Adafruit_DRV2605 drv;
NimBLEServer*      g_server = nullptr;
NimBLECharacteristic* g_char = nullptr;

// ---- Effect maps (ERM library) ----
// Pick simple, distinct cues for each pattern:
uint8_t effectFor(uint8_t pattern, uint8_t strength) {
  // We vary the effect ID by "strength" to get longer/stronger pulses.
  // See DRV2605L ERM effect table (Adafruit lib):
  // 1: strong click, 8: sharp tick, 14: double click, 47: ramp up
  switch (pattern) {
    case 1: // ALARM
      return (strength >= 2) ? 47 : 1;      // ramp up for strong, click for light
    case 2: // LATE
      return (strength >= 2) ? 14 : 8;      // double click vs tick
    case 3: // MISSED
      return 47;                            // always strong ramp
    case 4: // SHORT / snooze
    default:
      return 8;                             // short tick
  }
}

void playHaptic(uint8_t pattern, uint8_t repeats, uint8_t strength) {
  repeats   = constrain(repeats, 1, 5);
  strength  = constrain(strength, 0, 3);

  uint8_t eff = effectFor(pattern, strength);

  // program 1–2 waveforms per burst to feel richer on higher strength
  for (uint8_t r = 0; r < repeats; r++) {
    drv.setWaveform(0, eff);     // slot 0 = main effect
    if (strength >= 3) {
      // add a second short click for "very strong"
      drv.setWaveform(1, 1);
      drv.setWaveform(2, 0);     // end
    } else {
      drv.setWaveform(1, 0);     // end
    }
    drv.go();

    // spacing between bursts; longer for higher strength
    delay(220 + 120 * strength);
  }
}

class VibWriteCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    const std::string &v = c->getValue();
    if (v.size() < 3) return;
    uint8_t pattern  = static_cast<uint8_t>(v[0]);
    uint8_t repeats  = static_cast<uint8_t>(v[1]);
    uint8_t strength = static_cast<uint8_t>(v[2]);
    playHaptic(pattern, repeats, strength);
  }
};

void setupBLE() {
  NimBLEDevice::init("GuardianRX-Band");           // shows in scanners
  NimBLEDevice::setPower(ESP_PWR_LVL_P6);          // moderate TX power
  g_server = NimBLEDevice::createServer();

  NimBLEService* svc = g_server->createService(SVC_UUID);
  g_char = svc->createCharacteristic(
      CHR_UUID,
      NIMBLE_PROPERTY::WRITE_NR  // Write Without Response
  );
  g_char->setCallbacks(new VibWriteCB());
  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06); // defaults are fine; keep power low
  adv->start();
}

void setup() {
  // I2C + DRV2605L
  Wire.begin(SDA, SCL);
  if (!drv.begin()) {
    // If DRV2605L not found, still advertise; just skip haptics
  }
  drv.selectLibrary(1);                 // ERM
  drv.setMode(DRV2605_MODE_INTTRIG);    // internal trigger (fire on .go())

  // BLE
  setupBLE();
}

void loop() {
  // Low duty loop; for power saving you can use light sleep later.
  delay(800);
}
