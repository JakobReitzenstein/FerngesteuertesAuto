#include <esp_now.h> 
#include <WiFi.h>
#include <ESP32Servo.h>

typedef struct struct_message {
  int beschleunigung;
  int lenken;
  int lichter;
  int blinker;
  int ton;
  int hupe;
} struct_message;

struct_message IN_data;
Servo myServo;

int servoPin = 18;
int dataValid;

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

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(ONDataRecv));

  myServo.attach(servoPin);
}


String state = "fetch_Data";

int beschleunigung;
int lenken;
int lichter;
int blinker;
int ton;
int hupe; 

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
    int winkel = (IN_data.lenken * 180) / 100;
    Serial.print("Winkel: ");
    Serial.println(winkel);
    myServo.write(winkel);
    state = "fetch_Data";
    //delay(10);

    // Steuerung Beschleunigung


    // Steurung Lichter/Blinker

    // Steuerung ton/Hupe

  }



}