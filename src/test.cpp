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

  ledcSetup(0, 500, 8); // Channel 0, 5kHz frequency, 8-bit resolution
  ledcAttachPin(MotorPin, 0);
}

void loop() {
  static int inputValue = 0;

  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');
    inputValue = inputString.toInt();
    Serial.print("Eingegebene Zahl: ");
    Serial.println(inputValue);
    beschleunigung = inputValue;
    beschleunigungControl();
  }
  
  /*
  // setting all pins at the same time to either HIGH or LOW
  sr.setAllHigh(); // set all pins HIGH
  Serial.println("All pins set HIGH");
  delay(1000);
  
  sr.setAllLow(); // set all pins LOW
  Serial.println("All pins set LOW");
  delay(1000);

  sr.set(0, HIGH); // set pin 0 HIGH
  Serial.println("Pin 0 set HIGH");
  delay(1000);
  sr.set(0, LOW); // set pin 0 LOW
  Serial.println("Pin 0 set LOW");
  sr.set(1, HIGH); // set pin 1 HIGH
  Serial.println("Pin 1 set HIGH");   
  delay(1000);
  sr.set(1, LOW); // set pin 1 LOW
  Serial.println("Pin 1 set LOW");
  delay(1000);
  sr.set(2, HIGH); // set pin 2 HIGH
  Serial.println("Pin 2 set HIGH");
  delay(1000);
  sr.set(2, LOW); // set pin 2 LOW
  Serial.println("Pin 2 set LOW");
  delay(1000);
  sr.set(3, HIGH); // set pin 3 HIGH
  Serial.println("Pin 3 set HIGH");
  delay(1000);
  sr.set(3, LOW); // set pin 3 LOW
  Serial.println("Pin 3 set LOW");
  delay(1000);
  /*
  
  // setting single pins
  for (int i = 0; i < 8; i++) {
    Serial.print("Setting pin ");
    Serial.println(i);  
    sr.set(i, HIGH); // set single pin HIGH
    delay(250); 
  }
  */
  /*
  // set all pins at once
  uint8_t pinValues[] = { B10101010 }; 
  sr.setAll(pinValues); 
  delay(1000);

  
  // read pin (zero based, i.e. 6th pin)
  uint8_t stateOfPin5 = sr.get(5);
  sr.set(6, stateOfPin5);


  // set pins without immediate update
  sr.setNoUpdate(0, HIGH);
  sr.setNoUpdate(1, LOW);
  // at this point of time, pin 0 and 1 did not change yet
  sr.updateRegisters(); // update the pins to the set values
  */
}  