#include <esp_now.h> 
#include <WiFi.h>
#include <ESP32Servo.h>
#include <ShiftRegister74HC595.h>
#include "esp_random.h"

// Receiver ESP32 MAC Address
uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xF2, 0x9C};  // <--- replace if needed

ShiftRegister74HC595<1> sr(12, 14, 13); // Data, Clock, Latch

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
int Blinkerlinks = 25;
int BlinkerRechts = 26;
int dataValid;
int checknum = 0;
int checkOk = 0;

void ONDataRecv(const uint8_t * mac, const uint8_t * incomingData, int len) {
  memcpy(&IN_data, incomingData, sizeof(IN_data));
  Serial.print("Bytes received: ");
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
  dataValid = 1;
}

void lenkenControl() {
  int winkel = (lenken * 180) / 100;
  Serial.print("Winkel: ");
  Serial.println(winkel);
  myServo.write(winkel);
}

void beschleunigungControl() {
  ledcWrite(MotorPin, beschleunigung*255/100);
}

void lichterControl() {
  if (blinker == 1) {
    digitalWrite(Blinkerlinks, HIGH);
    delay(1);
    digitalWrite(Blinkerlinks, LOW);
  }
  if (blinker == 2) {
    digitalWrite(BlinkerRechts, HIGH);
    delay(1);
    digitalWrite(BlinkerRechts, LOW);
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
  pinMode(Blinkerlinks, OUTPUT);
  pinMode(BlinkerRechts, OUTPUT);

  ledcAttachPin(MotorPin, 0);
  ledcSetup(0, 5000, 8); // Channel 0, 5kHz frequency, 8-bit resolution
}

void loop() {
  if (state == "fetch_Data") {
    if (dataValid == 1) {
      beschleunigung  = IN_data.beschleunigung;
      lenken          = IN_data.lenken;
      lichter         = IN_data.lichter;
      blinker         = IN_data.blinker;
      ton             = IN_data.ton;
      hupe            = IN_data.hupe;
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
    state = "callback";
  }

  if (state == "sendcallback") {
    checknum = esp_random();
    OUT_data.check = checknum;
    esp_now_send(receiverAddress, (uint8_t *)&OUT_data, sizeof(OUT_data));
    Serial1.println("Callback sent");
    state = "waitForCallback"; 
  }

  if (state == "waitForCallback") {
    if (IN_data.check == checknum) {
      Serial.println("Check OK");
      state = "fetch_Data";
    } else {
      Serial.println("Check failed");
    }
  }
}