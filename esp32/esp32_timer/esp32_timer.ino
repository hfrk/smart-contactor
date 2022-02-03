#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define MQTT_DEBUG

#define PUB_INTERVAL 100
#define MQTT_CONNECT_ATTEMPT_INTERVAL 5000
unsigned long last_pub = 0;
unsigned long last_mqtt_connect_attempt = 0;

// timer stuff
unsigned long timer_last;
unsigned long TIMER_SETTIME;
#define MINUTE 60000
bool isTimerOn = false;

const int RELAY_PIN = LED_BUILTIN; // Relay
#define RELAY_PIN_ACTIVE HIGH
bool state = false;

// Ganti dengan ssid dan password wifi
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "hfrk.de";
const int mqtt_port = 1883;
uint8_t mqtt_connect_tries = 0;

const char* ID = "ESP2";
// Subscribe topics:
const char* SUB_command = "command/ESP2";
const char* SUB_timer = "timer/ESP2";
const char* SUB_global_timer = "TIMER";
// Publish to topic:
const char* PUB_status = "status/ESP2";

char data[80];
bool immediate_update = false;
WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_connect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(ID)) {
      mqtt_connect_tries = 0;
      client.subscribe(SUB_command);
      client.subscribe(SUB_timer);
      client.subscribe(SUB_global_timer);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(SUB_command);
      Serial.println(SUB_timer);
      Serial.println(SUB_global_timer);
    }
    Serial.println('\n');
    last_mqtt_connect_attempt = millis();
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

  if (strncmp(topic, SUB_global_timer, strlen(SUB_global_timer)) == 0) {
    // TIMER logic
    long payload_minute = String((char*)payload).toInt();
    if (payload_minute > 0) {
      TIMER_SETTIME = payload_minute;
      timer_last = millis();
      isTimerOn = true;
    }
  }
  else if (strncmp(topic, SUB_command, strlen(SUB_command)) == 0) {
    // Relay
    immediate_update = true;
    if (strncmp((char*)payload, "true", 4) == 0)
      state = true;
    else
      state = false;
  }
  else if (strncmp(topic, SUB_timer, strlen(SUB_timer)) == 0) {
    long payload_millis = String((char*)payload).toInt();
    if (payload_millis > 0) {
      TIMER_SETTIME = payload_millis;
      timer_last = millis();
      isTimerOn = true;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
  delay(2000);
}

void loop() {
  // Turn relay on/off
  digitalWrite(RELAY_PIN, state ? RELAY_PIN_ACTIVE : !RELAY_PIN_ACTIVE);

  // Timer turn off logic
  if ((isTimerOn) && (millis() - timer_last > TIMER_SETTIME)) {
    state = false;
    isTimerOn = false;
  }

  if (!client.connected()
      && mqtt_connect_tries < 10
      && millis() - last_mqtt_connect_attempt > MQTT_CONNECT_ATTEMPT_INTERVAL) {
    mqtt_connect_tries++;
    mqtt_connect();
  }

  // Publish status
  if (immediate_update || millis() - last_pub > PUB_INTERVAL) {
    String payload = state ? "true" : "false";
    payload.toCharArray(data, (payload.length() + 1));
    client.publish(PUB_status, data);
    last_pub = millis();
    immediate_update = false;
  }

  client.loop();
}
