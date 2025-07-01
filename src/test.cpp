/*
  ShiftRegister74HC595 - Library for simplified control of 74HC595 shift registers.
  Developed and maintained by Timo Denk and contributers, since Nov 2014.
  Additional information is available at https://timodenk.com/blog/shift-register-arduino-library/
  Released into the public domain.
*/
#include <Arduino.h>
// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
//ShiftRegister74HC595<1> sr(13, 14, 16);

int MotorPin = 33;
int beschleunigung = 0;

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

  //ledcSetup(0, 500, 8); // Channel 0, 5kHz frequency, 8-bit resolution
  //ledcAttachPin(MotorPin, 0);
}

void loop() {
  /*static int inputValue = 0;

  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');
    inputValue = inputString.toInt();
    Serial.print("Eingegebene Zahl: ");
    Serial.println(inputValue);
    beschleunigung = inputValue;
    beschleunigungControl();
  }
  */

  Serial.println("success");
  delay(1000);
  
}  