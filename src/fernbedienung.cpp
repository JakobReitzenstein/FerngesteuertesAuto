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
uint8_t receiverAddress[] = {0xD4, 0x8A, 0xFC, 0x5F, 0xF2, 0x9C};  // replace if needed

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

int testY = 2100;
int testValid = 1;

void test() {
  while (testValid) {
    // Hochzählen von 2100 bis 4000
    for (int i = 0; i<10; i++) {
      data.beschleunigung = 0;
      data.lenken = 50; // Dummy value for lenken
      data.lichter = 1; // Dummy value for lichter  
      data.blinker = 0; // Dummy value for blinker
      data.ton = 0;     // Dummy value for ton
      data.hupe = 0;    // Dummy value for hupe
      if (i == 5)
      {
        data.blinker = 1;
      }
      
      esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
      delay(38);
    }

    for (testY = 2000; testY <= 4000 && testValid; testY += 50) {
      data.beschleunigung = scaleWithDeadzone(testY);
      data.lichter = 2; 
      
      Serial.print("testY: ");
      Serial.println(testY);
      data.blinker = 0;
      if (testY == 2200) {
        data.blinker = 2; // Set blinker on at 2200
      }
      esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
      delay(38);
    }
    
    // Zurücksetzen auf 2100
    testY = 2000;
    data.blinker = 2;
    Serial.println("Reset auf 2100");
    data.beschleunigung = scaleWithDeadzone(testY);
    esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
    delay(200);

    // Runterzählen von 2000 bis 0
    for (testY = 2000; testY >= 0 && testValid; testY -= 50) {
      data.beschleunigung = scaleWithDeadzone(testY);
      esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
      data.lichter = 3; // Dummy value for lichter
      Serial.print("testY: ");
      Serial.println(testY);
      delay(38);
    }
    // Wieder auf 2000 setzen
    testY = 2000;
    Serial.println("Wieder auf 2000");
    data.beschleunigung = scaleWithDeadzone(testY);
    esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
    delay(200);
    testValid = 0;

    for (int lenken = 50; lenken <= 100; lenken += 5) {
      data.lenken = lenken;
      esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
      delay(38);
    }
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

  test();
}

void loop() {
  /*int xRaw = analogRead(xPin);
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

  delay(40);
  */
 delay(10000);
}
