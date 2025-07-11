#include <Arduino.h>
int MotorPin = 33;
int beschleunigung = 0;
int CCInPin = 34;

void beschleunigungControl() {
  /*
  Beschleunigung: PWM Signal an MotorPin ausgeben
  */
  Serial.print("Beschleunigung: ");
  Serial.println(beschleunigung*255/100);
  ledcWrite(0, beschleunigung*255/100);
}

void setup(){
  Serial.begin(115200);

  ledcSetup(0, 50, 8); // Channel 0, 50Hz frequency, 8-bit resolution
  ledcAttachPin(MotorPin, 0);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    int value = input.toInt();
    if (value >= 0 && value <= 100) {
      beschleunigung = value;
      Serial.print("Neue Beschleunigung: ");
      Serial.println(beschleunigung);
      ledcWrite(0, beschleunigung * 255 / 100);
    } else {
      Serial.println("Bitte eine Zahl von 0 bis 100 eingeben!");
    }
  }
}