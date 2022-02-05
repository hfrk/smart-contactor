#include <ESP8266WiFi.h>
#include <espnow.h>
#define LED_PIN_ACTIVE LOW

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xAC, 0x67, 0xB2, 0x38, 0x1B, 0xBC};

// Structure example to send data
// Must match the receiver structure
typedef struct {
  uint8_t macAddress[6];
  uint8_t id;
  float temperature_data;
  float pressure_data;
  bool light_ok;
  unsigned long int waktu;
} ESP32_Slave_Device;

// Create a struct_message called myData
ESP32_Slave_Device thisESP;

unsigned long lastTime = 0;  
unsigned long timerDelay = 5000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
// callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, unsigned char len) {
  Serial.print("Bytes received: ");
  Serial.println(len);
  for(int idx = 0; idx < 6; idx++) {
    Serial.print(mac[idx], HEX);
    idx == 5 ? Serial.print("\n") : Serial.print(":");
  }
  for(int idx = 0; idx < len; idx++) Serial.print((char)incomingData[idx]);
  Serial.println();

  if (strncmp((char*)incomingData, "true", 4) == 0) {
    digitalWrite(LED_BUILTIN, LED_PIN_ACTIVE);
  }
  if (strncmp((char*)incomingData, "false", 5) == 0) {
    digitalWrite(LED_BUILTIN, !LED_PIN_ACTIVE);
  }
}
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.channel());
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 7, NULL, 0);
  WiFi.macAddress(thisESP.macAddress);
  thisESP.id = 82;
  thisESP.temperature_data = random(20,40);
  thisESP.pressure_data = random(99325, 101325);
  thisESP.light_ok = random(false, true);
  thisESP.waktu = millis();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &thisESP, sizeof(thisESP));

    lastTime = millis();
  }
}
