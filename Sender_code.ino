#include <esp_now.h>
#include <WiFi.h>

int scaleWithDeadzone(int value) {
  if (value < 1900) {
    return map(value, 0, 1899, 0, 49);
  } else if (value <= 2100) {
    return 50;
  } else {
    return map(value, 2101, 4095, 51, 100);
  }
}


// Joystick pins — verified with your working setup
const int xPin = 35;       // X-axis (VRx)
const int yPin = 33;       // Y-axis (VRy)
const int buttonPin = 32;  // Button (SW)

// Receiver ESP32 MAC Address
uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xF2, 0x9C};  // <--- replace if needed

// Data structure for sending values
typedef struct struct_message {
  int x;
  int y;
  int button;
} struct_message;

struct_message data;

void setup() {
  Serial.begin(115200);

  // Init WiFi in station mode for ESP-NOW
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer");
    return;
  }

  // Button setup
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  int xRaw = analogRead(xPin);
  int yRaw = analogRead(yPin);
  int buttonState = digitalRead(buttonPin);

  data.x = scaleWithDeadzone(xRaw);
  data.y = scaleWithDeadzone(yRaw);
  data.button = (buttonState == LOW) ? 1 : 0;

  esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));

  Serial.print("X: ");
  Serial.print(data.x);
  Serial.print("\tY: ");
  Serial.print(data.y);
  Serial.print("\tButton: ");
  Serial.println(data.button);

  delay(100);
}

