#include <esp_now.h>
#include <WiFi.h>

#define LED_PIN_ACTIVE HIGH

//uint8_t broadcastAddress[] = {0xAC, 0x67, 0xB2, 0x38, 0x1B, 0xBC};
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Bytes received: ");
  Serial.println(len);
  for(int idx = 0; idx < 6; idx++) {
    Serial.print(mac[idx], HEX);
    idx == 5 ? Serial.print("\n") : Serial.print(":");
  }
  for(int idx = 0; idx < len; idx++) Serial.print((char)incomingData[idx]);
  Serial.println();
  if (strncmp((char*)incomingData, "echo", 4) == 0) {
    uint8_t reply[] = "echo";
    esp_err_t tes = esp_now_send(broadcastAddress, reply, sizeof(reply));
  }
  if (strncmp((char*)incomingData, "true", 4) == 0) {
    digitalWrite(LED_BUILTIN, LED_PIN_ACTIVE);
  }
  if (strncmp((char*)incomingData, "false", 5) == 0) {
    digitalWrite(LED_BUILTIN, !LED_PIN_ACTIVE);
  }
}
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {

}
