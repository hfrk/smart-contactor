#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
unsigned long int last_time = 0;
uint8_t count = 0;
#define LED_PIN_ACTIVE HIGH
uint8_t channel = 0;
bool master_address_exist = false;
uint8_t masterAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
typedef struct {
  uint8_t macAddress[6];
  uint8_t id;
  float temperature_data;
  float pressure_data;
  bool light_ok;
  unsigned long int waktu;
} ESP32_Slave_Device;
ESP32_Slave_Device thisESP;

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

    if (!esp_now_is_peer_exist(mac)) {
      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, mac, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
  
      // Add peer
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
      }
      esp_now_del_peer(masterAddress);
      memcpy(masterAddress, mac, 6);
    }
    esp_now_send(masterAddress, reply, sizeof(reply));
    master_address_exist = true;
  }
  if (strncmp((char*)incomingData, "true", 4) == 0) {
    digitalWrite(LED_BUILTIN, LED_PIN_ACTIVE);
  }
  if (strncmp((char*)incomingData, "false", 5) == 0) {
    digitalWrite(LED_BUILTIN, !LED_PIN_ACTIVE);
  }
}
int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  channel = getWiFiChannel("ESPMaster");
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
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
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = channel;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  WiFi.macAddress(thisESP.macAddress);
  thisESP.id = 1;
  thisESP.temperature_data = random(20,40);
  thisESP.pressure_data = random(99325, 101325);
  thisESP.light_ok = random(false, true);
  thisESP.waktu = millis();
}

void loop() {
  if (millis() - last_time > 5000 && master_address_exist) {
    last_time = millis();
    thisESP.temperature_data = float(random(20000,40000))/1000.0;
    thisESP.pressure_data = float(random(9932500, 10132500))/100.0;
    thisESP.light_ok = random(0, 2);
    thisESP.waktu = millis();
    esp_now_send(masterAddress, (uint8_t*)&thisESP, sizeof(thisESP));
    count++;
  }
}
