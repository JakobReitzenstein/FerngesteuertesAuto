#include <esp_now.h> 
#include <WiFi.h>
#include <ESP32Servo.h>
#include <ShiftRegister74HC595.h>
#include "esp_random.h"

// Receiver ESP32 MAC Address
uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xF2, 0x9C};  // <--- replace if needed

ShiftRegister74HC595<1> sr(13, 14, 16); // Data, Clock, Latch

String state = "fetch_Data";

int beschleunigung;
int lenken;
int lichter;
int blinker;
int ton;
int hupe; 

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


struct_message_IN   IN_data;
struct_message_OUT  OUT_data;
Servo myServo;

int servoPin = 18;
int MotorPin = 33;
int BlinkerlinksPin = 25;
int BlinkerRechtsPin = 26;
int FernlichtPin = 4;
int BremslichtPin = 19;

int dataValid;
int checknum = 0;
int checkOk = 0;

void ONDataRecv(const uint8_t * mac, const uint8_t * incomingData, int len) {
  memcpy(&IN_data, incomingData, sizeof(IN_data));
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
  dataValid = 1;
}

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
  ledcWrite(0, beschleunigung*255/100);
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

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(ONDataRecv));

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer");
    return;
  }

  myServo.attach(servoPin);
  pinMode(BlinkerlinksPin, OUTPUT);
  pinMode(BlinkerRechtsPin, OUTPUT);

  ledcSetup(0, 5000, 8); // Channel 0, 5kHz frequency, 8-bit resolution
  ledcAttachPin(MotorPin, 0);
}

void loop() {
  if (state == "initialize") {
    // Initialisierung der Verbindung zut Fernbedinung
    // Senden einer random Zahl, wenn die zahl wieder empfangen wird, ist die Verbindung hergestellt
    checknum = esp_random();
    OUT_data.check = checknum;
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    Serial.println("Initialize sent");
    state = "fetch_Data";
  }

  if (state == "fetch_Data") {
    if (dataValid == 1) {
      beschleunigung  = IN_data.beschleunigung;
      lenken          = IN_data.lenken;
      lichter         = IN_data.lichter;
      blinker         = IN_data.blinker;
      ton             = IN_data.ton;
      hupe            = IN_data.hupe;
      Serial.println("Daten erhalten");
      state = "control";
    }
    dataValid = 0;
  }
  if (state == "control") {

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

  if (state == "sendcallback") {
    checknum = esp_random();
    OUT_data.check = checknum;
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    Serial.println("Callback sent");
    state = "waitForCallback"; 
  }

  if (state == "waitForCallback") {
    /*Serial.print("checknum: ");
    Serial.println(checknum);
    Serial.print("IN_data.check: ");
    Serial.println(IN_data.check);
    */
    // TODO: wenn callback zu lange dauert, StatusLed setzen und Fehlerbehandlung

    if (IN_data.check == checknum) {
      Serial.println("Check OK");
      state = "fetch_Data";
    }
  }

}