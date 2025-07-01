#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // MAC-Adresse ausgeben
  Serial.println("ESP32 MAC-Adresse:");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // nichts
}
