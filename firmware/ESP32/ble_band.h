/*
  Guardian RX — BLE Wristband Helper (Central side)
  Code by Solehin Rizal
  Drop this header into: firmware/ArduinoIDE/GuardianRX/ble_band.h

  Requires:
    - NimBLE-Arduino (by h2zero)

  GATT (must match the wristband):
    Service UUID:        6E400001-B5A3-F393-E0A9-E50E24DCCA9E
    Characteristic UUID: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E (Write Without Response)
    Payload (3 bytes): [pattern, repeats, strength]
*/

#pragma once
#include <NimBLEDevice.h>

// ======== Config (edit if you changed names/UUIDs) ========
static const char* GDRX_BAND_NAME   = "GuardianRX-Band";  // device name to match (optional)
static const char* GDRX_SVC_UUID    = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
static const char* GDRX_CHAR_UUID   = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

// BLE TX power (range vs current draw): ESP_PWR_LVL_N0..P9
#ifndef GDRX_BLE_TX_POWER
#define GDRX_BLE_TX_POWER ESP_PWR_LVL_P6
#endif

namespace gdrx_ble {

static NimBLEAddress g_bandAddr;   // cached target address
static bool          g_haveAddr = false;
static uint32_t      g_lastSeenMs = 0;

// ----- Scanner callback: caches first match then stops scan -----
class BandScanCB : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* adv) override {
    const bool svcMatch = adv->isAdvertisingService(NimBLEUUID(GDRX_SVC_UUID));
    const bool nameMatch = adv->haveName() && adv->getName() == std::string(GDRX_BAND_NAME);
    if (svcMatch || nameMatch) {
      g_bandAddr = adv->getAddress();
      g_haveAddr = true;
      g_lastSeenMs = millis();
      NimBLEDevice::getScan()->stop();
    }
  }
};

// ----- Init BLE stack for Central use -----
inline void init(const char* centralName = "GuardianRX-Pillbox") {
  NimBLEDevice::init(centralName);
  NimBLEDevice::setPower(GDRX_BLE_TX_POWER);
  NimBLEDevice::setSecurityAuth(false, false, false); // no bonding by default
  // (Optional) limit connections to 1 to reduce RAM:
  NimBLEDevice::setClientAppearance(0);
}

// ----- Scan for wristband (non-blocking feel, returns true if found) -----
inline bool findBand(uint32_t scanMs = 4000, bool active = true) {
  if (g_haveAddr && (millis() - g_lastSeenMs) < 60 * 1000UL) return true; // cache is fresh
  NimBLEScan* scan = NimBLEDevice::getScan();
  static BandScanCB cb;
  scan->setAdvertisedDeviceCallbacks(&cb, false);
  scan->setActiveScan(active);
  scan->setInterval(45);
  scan->setWindow(30);
  g_haveAddr = false;
  scan->start(scanMs, false);  // false = don't call onComplete
  return g_haveAddr;
}

// ----- Internal: connect and write 3-byte command, then disconnect -----
inline bool writeCommand(uint8_t pattern, uint8_t repeats, uint8_t strength, uint16_t opTimeoutMs = 1500) {
  if (!g_haveAddr && !findBand()) return false;

  NimBLEClient* client = NimBLEDevice::createClient();
  client->setConnectTimeout(opTimeoutMs);
  bool ok = false;

  if (client->connect(g_bandAddr)) {
    NimBLERemoteService* svc = client->getService(GDRX_SVC_UUID);
    if (svc) {
      NimBLERemoteCharacteristic* chr = svc->getCharacteristic(GDRX_CHAR_UUID);
      if (chr && chr->canWriteNoResponse()) {
        uint8_t buf[3] = { pattern, repeats, strength };
        ok = chr->writeValue(buf, sizeof(buf), true); // true = no response
      }
    }
    client->disconnect();
  } else {
    // connection failed; forget cached address so next call rescans
    g_haveAddr = false;
  }

  NimBLEDevice::deleteClient(client);
  return ok;
}

// ----- Public API: vibrate patterns (non-blocking-ish, short op) -----
// Returns true if the BLE write was attempted and succeeded.
inline bool vibrate(uint8_t pattern, uint8_t repeats, uint8_t strength) {
  // clamp to sane ranges
  if (repeats == 0) repeats = 1;
  if (repeats > 5)  repeats = 5;
  if (strength > 3) strength = 3;
  // Use a short operation timeout so alerts never stall UI
  return writeCommand(pattern, repeats, strength, 1500);
}

// Convenience helpers
inline bool vibrateAlarm() { return vibrate(1, 2, 2); }   // Due
inline bool vibrateLate()  { return vibrate(2, 2, 1); }   // Late
inline bool vibrateMissed(){ return vibrate(3, 3, 2); }   // Missed
inline bool vibrateShort() { return vibrate(4, 1, 1); }   // Snooze/nudge

} // namespace gdrx_ble
