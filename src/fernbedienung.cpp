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

// Joystick pins
const int xPin = 35;       // X-axis (VRx)
const int yPin = 33;       // Y-axis (VRy)
const int buttonPin = 32;  // Button (SW)

// Receiver ESP32 MAC Address
uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xDF, 0x94};  // replace if needed

// Structure to send
typedef struct struct_message_out {
  int beschleunigung;
  int lenken;
  int lichter;
  int blinker;
  int ton;
  int hupe;
  int check;
} struct_message_out;

// Structure to receive
typedef struct struct_message_in {
  int check;
} struct_message_in;

struct_message_out data;
struct_message_in receivedData;

// Callback function for receiving data
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_message_in)) {
    memcpy(&receivedData, incomingData, sizeof(struct_message_in));
    data.check = receivedData.check;
    Serial.print("Received check value: ");
    Serial.println(data.check);
  } else {
    Serial.println("Received unknown data");
  }
}

void setup() {
  Serial.begin(115200);

  // Init WiFi in station mode
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

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);

  // Button setup
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  int xRaw = analogRead(xPin);
  int yRaw = analogRead(yPin);
  int buttonState = digitalRead(buttonPin);

  data.beschleunigung = scaleWithDeadzone(xRaw);
  data.lenken = scaleWithDeadzone(yRaw);
  data.hupe = (buttonState == LOW) ? 1 : 0;

  esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));

  Serial.print("X: ");
  Serial.print(xRaw);
  Serial.print("\tY: ");
  Serial.print(yRaw);
  Serial.print("\tButton: ");
  Serial.print(buttonState);
  Serial.print("\tCheck: ");
  Serial.println(data.check);

  delay(100);
}
