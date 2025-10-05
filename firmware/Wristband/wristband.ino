/*
  Guardian RX — Wristband
  Code by Solehin Rizal
  Board: Seeed XIAO ESP32C3
  Motor Driver : MAKER DRIVE by Cytron (PWM)

*/

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "test";
const char* password = "test123456";

#define VIB_PIN D10  // vibration motor pin

WebServer server(80);

void handleVibrate() {
  Serial.println("Vibration trigger received!");
  digitalWrite(VIB_PIN, HIGH);
  delay(1500);                 // vibrate 1.5s
  digitalWrite(VIB_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(VIB_PIN, OUTPUT);
  digitalWrite(VIB_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected!");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  server.on("/vibrate", handleVibrate);
  server.begin();
  Serial.println("Server started.");
}

void loop() {
  server.handleClient();
}
