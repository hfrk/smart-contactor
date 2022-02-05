#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
int32_t channel = 0;
//////////////////////////////////////////////
// MQTT stuff
#define MQTT_DEBUG
#define PUB_INTERVAL 6789
#define MQTT_CONNECT_ATTEMPT_INTERVAL 5000
unsigned long last_pub = 0;
unsigned long last_mqtt_connect_attempt = 0;
// Ganti dengan ssid dan password wifi
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "hfrk.de";
const int mqtt_port = 1883;
uint8_t mqtt_connect_tries = 0;
const char* ID = "ESPMaster";
// Subscribe topics:
const char* SUB_command = "command/ESPMaster";
// Publish to topic:
const char* PUB_status = "status/ESPMaster";
const char* PUB_data = "data";
char data[2560];
WiFiClient espClient;
PubSubClient client(espClient);
// End of MQTT stuff
//////////////////////////////////////////////

//////////////////////////////////////////////
// ESP-NOW stuff
typedef struct {
  uint8_t macAddress[6];
  uint8_t id;
  float temperature_data;
  float pressure_data;
  bool light_ok;
  unsigned long int waktu;
} ESP32_Slave_Device;

ESP32_Slave_Device slaves[20];
uint8_t slave_count = 0;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
char transmit[250];
byte idx_tx = 0;
// End of ESP-NOW stuff
//////////////////////////////////////////////

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
  if (strncmp((char*)incomingData, "echo", 4) == 0) {
    for(int idx = 0; idx < len; idx++) Serial.print((char)incomingData[idx]);
    Serial.println();
    if (!esp_now_is_peer_exist(mac)) {
      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, mac, 6);
      peerInfo.channel = channel;
      peerInfo.encrypt = false;
  
      // Add peer
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
      }
      memset(&slaves[slave_count], 0, sizeof(slaves[slave_count]));
      memcpy(slaves[slave_count].macAddress, mac, 6);
      slave_count++;
      slave_count%=20;
    }
  }
  else {
    ESP32_Slave_Device foobar;
    memset(&foobar, 0, sizeof(foobar));
    memcpy(&foobar, incomingData, sizeof(foobar));
    for (int count = 0; count < slave_count; count++) {
      if (memcmp(foobar.macAddress, slaves[count].macAddress, sizeof(foobar.macAddress)) == 0) {
        slaves[count].temperature_data = foobar.temperature_data;
        slaves[count].pressure_data = foobar.pressure_data;
        slaves[count].light_ok = foobar.light_ok;
        slaves[count].waktu = foobar.waktu;
        break;
      }
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
#ifdef MQTT_DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] [");
  Serial.print(length);
  Serial.print(" byte(s)] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif

  if (strncmp(topic, SUB_command, strlen(SUB_command)) == 0) {
    // Command logic
    if (strncmp((char*)payload, "scan", 4) == 0) {
      uint8_t echo[] = "echo\n";
      esp_now_send(broadcastAddress, echo, sizeof(echo));
    }
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
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  
  channel = getWiFiChannel(ssid);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  WiFi.begin(ssid, password, channel);
  WiFi.softAP("ESPMaster", NULL, channel+1);
  Serial.printf("Connecting to %s", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = channel;
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  //uint8_t echo[] = "echo\n";
  //esp_now_send(broadcastAddress, echo, sizeof(echo));
}

void loop() {
  read_from_serial_and_broadcast_via_ESP_NOW();
  reconnect_if_mqtt_disconnected();

  // Publish status
  if (millis() - last_pub > PUB_INTERVAL) {
    String payload = String("MAC address: ") + WiFi.macAddress() + '\n';
    payload += "Connected peer(s): " + String(slave_count) + '\n';
    for (int idx = 0; idx < slave_count; idx++) {
      payload += String(idx+1) + ". MAC address: " + mac_addr_to_string(slaves[idx].macAddress) + '\n';
    }
    payload.toCharArray(data, (payload.length() + 1));
    client.publish(PUB_status, data);
    Serial.println(payload);
    payload = "";
    for (int count = 0; count < slave_count; count++){
      payload += "MAC address:" + mac_addr_to_string(slaves[count].macAddress) + '\n';
      payload += "Temperature:" + String(slaves[count].temperature_data) + '\n';
      payload += "Pressure:" + String(slaves[count].pressure_data) + '\n';
      payload += "Light:" + String(slaves[count].light_ok) + '\n';
      payload += "Millis:" + String(slaves[count].waktu) + '\n';
    }
    payload.toCharArray(data, (payload.length() + 1));
    client.publish(PUB_data, data);
    last_pub = millis();
  }

  client.loop();
}

String mac_addr_to_string(uint8_t* mac_addr) {
  String result = "";
  for (int idx = 0; idx < 6; idx++) {
    result += String(mac_addr[idx], HEX);
    if (idx < 5) result += ':'; 
  }
  result.toUpperCase();
  return result;
}

void read_from_serial_and_broadcast_via_ESP_NOW() {
  if (Serial.available()) {
    do {
      transmit[idx_tx] = Serial.read();
      idx_tx++;
    }
    while(Serial.available() > 0);
    
    if (transmit[idx_tx-1] == '\n') {
      // Send message via ESP-NOW
      Serial.print("Transmitting: "); Serial.println(transmit);
      esp_now_send(broadcastAddress, (uint8_t *) &transmit, sizeof(transmit[0])*idx_tx);
      while(idx_tx--) transmit[idx_tx] = '\0';
      idx_tx = 0;
    }
  }
}

void reconnect_if_mqtt_disconnected() {
  if (!client.connected()
      && mqtt_connect_tries < 10
      && millis() - last_mqtt_connect_attempt > MQTT_CONNECT_ATTEMPT_INTERVAL) {
    mqtt_connect_tries++;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(ID)) {
      mqtt_connect_tries = 0;
      client.subscribe(SUB_command);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(SUB_command);
    }
    Serial.println('\n');
    last_mqtt_connect_attempt = millis();
  }
}
