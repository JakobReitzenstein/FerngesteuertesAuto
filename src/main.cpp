#include <esp_now.h> 
#include <WiFi.h>
#include <ESP32Servo.h>
#include <ShiftRegister74HC595.h>
#include "esp_random.h"

// ****************************************************************
// Definieren der Datenstrukturen welche mit ESP-NOW gesendet werden

typedef struct struct_message_IN {
  int beschleunigung;
  int lenken;
  int lichter;
  int blinker;
  int ton;
  int hupe;
  int check;
} struct_message_IN;

typedef struct struct_message_OUT {
  int check;
} struct_message_OUT;

// ****************************************************************
// Definition der Variablen

int beschleunigung;
int lenken;
int lichter;
int blinker;
int ton;
int hupe; 
int dataValid = 0;
int checknum = 0;
int checkOk = 0;
long dauer = 0;
long callbacktime = 0;
struct_message_IN   IN_data;
struct_message_OUT  OUT_data;
Servo myServo;
String state = "fetch_Data"; 

// ****************************************************************
// MAC Address der Fernbedienung
//uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xF2, 0x9C};  

uint8_t receiverAddress[] = {0xFC, 0xB4, 0x67, 0xD1, 0x89, 0x70};  

// ****************************************************************
/*  Pin denfinitionen: 
    Servo Pin: GPIO 18
    Motor Pin: GPIO 33
    Blinker links Pin: GPIO 25
    Blinker rechts Pin: GPIO 26
    Fernlicht Pin: GPIO 4
    Bremslicht Pin: GPIO 19
    Abstandsensor Trigger Pin: GPIO 37
    Abstandsensor Echo Pin: GPIO 35
    Current check Pin: GPIO 34
    Shift Register Pins: 
    GPIO 13 (Data), 
    GPIO 14 (Clock), 
    GPIO 16 (Latch)
*/

int servoPin = 18;
int MotorVorPin = 32;
int MotorBackPin = 33;
int BlinkerlinksPin = 25;
int BlinkerRechtsPin = 26;
int FernlichtPin = 4;
int BremslichtPin = 19;
int AbstandsensensorTriggerPin = 17;
int AbstandsensensorEchoPin = 35;
int CCInPin = 34; 
ShiftRegister74HC595<1> sr(14, 12, 13); // Data, Clock, Latch

// ****************************************************************
// Funktion, welche bei Einkommenden Daten aufgerufen werden

void ONDataRecv(const uint8_t * mac, const uint8_t * incomingData, int len) {
  memcpy(&IN_data, incomingData, sizeof(IN_data));
  dataValid = 1;
  /*Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Beschleunigung: ");
  Serial.println(IN_data.beschleunigung);
  Serial.print("lenken: ");
  Serial.println(IN_data.lenken);
  Serial.print("lichter: ");
  Serial.println(IN_data.lichter);
  Serial.print("blinker: ");
  Serial.println(IN_data.blinker);
  Serial.print("ton: ");
  Serial.println(IN_data.ton);
  Serial.print("hupe: ");
  Serial.println(IN_data.hupe);
  */
}

// ****************************************************************
// Funktionen zur Steuerung der verschiedenen Komponenten

void lenkenControl() {
  /*
  int lenken ist ein Wert von 0 bis 100, der den Lenkwinkel steuert
  Servo dann auf berechneten Winkel einstellen
  */

  int winkel = (lenken * 180) / 100;
  Serial.print("Winkel: ");
  Serial.println(winkel);
  myServo.write(winkel);
}

void beschleunigungControl() {
  /*
  Beschleunigung: PWM Signal an MotorPin ausgeben
  */
  Serial.print("Beschleunigung: ");
  Serial.println(beschleunigung);
  if (beschleunigung < 50) {
    // Beschleunigung < 50, Rückwärtsfahrt
    int beschleunigungBack = (50 - beschleunigung) * 2; // Rückwärtsfahrt
    Serial.print("Rückwärtsfahrt, Beschleunigung: ");
    Serial.println(beschleunigungBack*255/100);
    ledcWrite(1, beschleunigungBack*255/100);
    ledcWrite(0, 0);
  }
  else if (beschleunigung > 50) {
    // Beschleunigung > 50, Vorwärtsfahrt
    int beschleunigungVor = (beschleunigung - 50) * 2; // Vorwärtsfahrt
    Serial.print("Vorwärtsfahrt, Beschleunigung: ");
    Serial.println(beschleunigungVor*255/100);
    ledcWrite(0, beschleunigungVor*255/100);
    ledcWrite(1, 0);
  }
  else {
    // Beschleunigung == 50, Motor aus
    ledcWrite(0, 0);
    ledcWrite(1, 0);
  }
}

void lichterControl() {
  /*
  Blinker: Trigger ausgeben, nach 1ms wieder LOW
  Blinker == 1 : Blinker links
  Blinker == 2 : Blinker rechts
  
  Lichter == 1: Bremslicht
  Lichter == 2: Fernlicht
  Lichter ==  3: Bremslicht und Fernlicht

  */
  if (blinker == 1) {
    digitalWrite(BlinkerlinksPin, HIGH);
    delay(1);
    digitalWrite(BlinkerlinksPin, LOW);
  }
  if (blinker == 2) {
    digitalWrite(BlinkerRechtsPin, HIGH);
    delay(1);
    digitalWrite(BlinkerRechtsPin, LOW);
  }

  if (lichter == 1) {
    digitalWrite(BremslichtPin, HIGH);
    digitalWrite(FernlichtPin, LOW);
  }
  else if (lichter == 2) {
    digitalWrite(FernlichtPin, HIGH);
    digitalWrite(BremslichtPin, LOW);
  }
  else if (lichter == 3) {
    digitalWrite(BremslichtPin, HIGH);
    digitalWrite(FernlichtPin, HIGH);
  } 
  else {
    digitalWrite(BremslichtPin, LOW);
    digitalWrite(FernlichtPin, LOW);
  }
}

void tonControl() {

}

// ****************************************************************
// Setup-Funktion, die einmalig beim Starten des Programms aufgerufen wird

void setup() {
  // Serielle Kommunikation starten
  Serial.begin(115200);
  // Alle Status-LEds setzen
  sr.setAllHigh();

  // Initalisierung ESP-NOW
  Serial.println("Start");
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(ONDataRecv));
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer");
    return;
  }

  // Servo und Lichter initialisieren
  myServo.attach(servoPin);
  pinMode(BlinkerlinksPin, OUTPUT);
  pinMode(BlinkerRechtsPin, OUTPUT);
  // Motor Pins initialisieren
  ledcSetup(0, 50, 8); // Channel 0, 50Hz frequency, 8-bit resolution
  ledcAttachPin(MotorVorPin, 0);
  ledcSetup(1, 50, 8); // Channel 1, 50Hz frequency, 8-bit resolution
  ledcAttachPin(MotorBackPin, 1);
}

// ****************************************************************
// Loop-Funktion, die kontinuierlich aufgerufen wird

void loop() {
  if (state == "initialize") {
    sr.setAllLow();
    sr.set(2, HIGH);
    // Initialisierung der Verbindung zut Fernbedinung
    // Senden einer random Zahl, wenn die zahl wieder empfangen wird, ist die Verbindung hergestellt
    dauer = millis();
    checknum = esp_random();
    OUT_data.check = checknum;
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    Serial.println("Initialize sent");
    state = "fetch_Data";
  }

  else if (state == "fetch_Data") {
    sr.setAllLow();
    sr.set(1, HIGH);
    if (dataValid == 1) {
      beschleunigung  = IN_data.beschleunigung;
      lenken          = IN_data.lenken;
      lichter         = IN_data.lichter;
      blinker         = IN_data.blinker;
      ton             = IN_data.ton;
      hupe            = IN_data.hupe;
      Serial.println("Daten erhalten");
      state = "CURRENT_CHECK";
      dauer = millis();
    }
    dataValid = 0;
  }

  else if (state == "CURRENT_CHECK") {
    sr.setAllLow();
    sr.set(2, HIGH);
    // Current Check Eingang checken, wenn >2 V dann alles aus
    int currentCheck = analogReadMilliVolts(CCInPin);
    if (currentCheck > 2000) { 
      Serial.println("Current check failed, spannung >2");
      state = "Error";
    }

    state = "control";
  }

  else if (state == "control") {
    sr.setAllLow();
    sr.set(3, HIGH);
    // Steuerung Lenken
    lenkenControl();
    // Steuerung Beschleunigung
    beschleunigungControl();
    // Steuerung Lichter/Blinker
    lichterControl();
    // Steuerung ton/Hupe
    tonControl();
    Serial.println("Control fertig");
    state = "sendcallback";
  }

  else if (state == "sendcallback") {
    // Cakllback Numer generieren und senden
    sr.setAllLow();
    sr.set(4, HIGH);
    checknum = esp_random();
    OUT_data.check = checknum;
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    Serial.println("Callback sent");
    callbacktime = millis();
    state = "waitForCallback"; 
  }

  else if (state == "waitForCallback") {
    // Warten auf Callback von der Fernbedienung
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    sr.setAllLow();
    sr.set(5, HIGH);

    if (millis() - callbacktime > 1000) {
      Serial.println("Callback fehlgeschlagen, Timeout");
      state = "Error";
      return;
    }

    if (IN_data.check == checknum) {
      Serial.println("Check OK");
      dauer = millis() - dauer;
      Serial.print("Zeit vergangen: ");
      Serial.println(dauer);
      state = "fetch_Data";
    }
  }

  else if (state == "Error") {
    sr.setAllLow();
    sr.set(6, HIGH);

    // Alle Signale Aus
    digitalWrite(BlinkerlinksPin, LOW);
    digitalWrite(BlinkerRechtsPin, LOW);
    digitalWrite(FernlichtPin, LOW);
    digitalWrite(BremslichtPin, LOW);
    ledcWrite(0, 0); // Motor vorwärts aus
    ledcWrite(1, 0); // Motor rückwärts aus
    myServo.write(90); // Servo auf Mittelstellung

    while (1) {
      Serial.println("Fehler");
      delay(1000);
    }
  }
}