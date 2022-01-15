#include <WiFi.h>
#include <PubSubClient.h>

//Logic ESP
const int RELAY_PIN1 = 2; //5;  //Relay
const int RELAY_PIN2 = 16; //4;  //Relay
const int buttonPin1 = 26; //3; //Push_button1
const int buttonPin2 = 25; //Push_button2
unsigned long button_time1 = 0;
unsigned long last_button_time1 = 0;
unsigned long button_time2 = 0;
unsigned long last_button_time2 = 0;
unsigned long last_pub = 0;
bool state1 = false;
bool state2 = false;

// timer stuff
unsigned long timer_last;
unsigned long TIMER_SETTIME;
#define MINUTE 60000
bool isTimerOn = false;

// Ganti dengan ssid dan password wifi
const char* ssid = "ssid";
const char* password = "password";

const char* server = "node-red.hfrk.de";
const char* mqtt_server = "20.63.210.119";
const char *ID = "ESP_Alex";  // Ganti dengan nama device
const char *TOPIC_1 = "ESP1/Lampu1/command";  // Ganti dengan topic yang ingin disubscribe
const char *TOPIC_2 = "ESP1/Lampu2/command";
const char *TOPIC_3 = "TIMER";
const char *PUB_TOPIC_1 = "ESP1/Lampu1";
const char *PUB_TOPIC_2 = "ESP1/Lampu2";

const int mqtt_port = 1883;
char data[80];

WiFiClient espClient;
PubSubClient client(espClient);

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(ID)) {
      client.subscribe(TOPIC_1);
      client.subscribe(TOPIC_2);
      client.subscribe(TOPIC_3);
      Serial.println("connected");
      Serial.print("Subcribed to: ");
      Serial.println(TOPIC_1);
      Serial.println(TOPIC_2);
      Serial.println(TOPIC_3);
      Serial.println('\n');
    } else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
/*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] [");
  Serial.print(length);
  Serial.print(" byte(s)] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
*/
  if (strncmp(topic, TOPIC_3, strlen(TOPIC_3)) == 0) {
    // TIMER logic
    if (strncmp((char*)payload, "150", 3) == 0) {
      // set timer ke 150 menit
      TIMER_SETTIME = 150 * MINUTE;
      timer_last = millis();
      isTimerOn = true;
      Serial.println("Timer diset ke 150 menit");
    }
    else if (strncmp((char*)payload, "120", 3) == 0) {
      // set timer ke 120 menit
      TIMER_SETTIME = 120 * MINUTE;
      timer_last = millis();
      isTimerOn = true;
      Serial.println("Timer diset ke 120 menit");
    }
    else if (strncmp((char*)payload, "90", 2) == 0) {
      // set timer ke 90 menit
      TIMER_SETTIME = 90 * MINUTE;
      timer_last = millis();
      isTimerOn = true;
      Serial.println("Timer diset ke 90 menit");
    }
    else if (strncmp((char*)payload, "60", 2) == 0) {
      // set timer ke 60 menit
      TIMER_SETTIME = 60 * MINUTE;
      timer_last = millis();
      isTimerOn = true;
      Serial.println("Timer diset ke 60 menit");
    }
  }
  else if (strncmp(topic, TOPIC_1, strlen(TOPIC_1)) == 0) {
    // Lampu 1
    if (strncmp((char*)payload, "true", 4) == 0)
      state1 = true;
    else
      state1 = false;
  }
  else if (strncmp(topic, TOPIC_2, strlen(TOPIC_2)) == 0) {
    // Lampu 2
    if (strncmp((char*)payload, "true", 4) == 0)
      state2 = true;
    else
      state2 = false;
  }
}

void setup() {
  //Logic ESP
  Serial.begin(115200);
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
   //Serial.begin(115200);
   WiFi.mode(WIFI_STA);                    //The WiFi is in station mode. The    other is the softAP mode
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }
   Serial.println("");  Serial.print("WiFi connected to: "); Serial.println(ssid);  Serial.println("IP address: ");  Serial.println(WiFi.localIP());
   client.setServer(mqtt_server, mqtt_port);
   client.setCallback(callback);
   delay(2000);
}

void loop() {
  // Logic ESP32
  switch (state1) {
  case true:
    digitalWrite(RELAY_PIN1, LOW);
    break;
  default:
    digitalWrite(RELAY_PIN1, HIGH);
    break;
  }

  switch (state2) {
  case true:
    digitalWrite(RELAY_PIN2, LOW);
    break;
  default:
    digitalWrite(RELAY_PIN2, HIGH);
    break;
  }

  // Timer logic
  if ((isTimerOn) && (millis() - timer_last > TIMER_SETTIME)) {
    state1 = false;
    state2 = false;
    isTimerOn = false;
  }

  if (!client.connected()) {
    mqttconnect();
  }
  // This sends off your payload. ganti dengan payload yang diinginkan

  if (millis() - last_pub > 100) {
    // Publish state1
    String payload = state1 ? "true" : "false";
    payload.toCharArray(data, (payload.length() + 1));
    client.publish(PUB_TOPIC_1, data);
    // Publish state2
    payload = state2 ? "true" : "false";
    payload.toCharArray(data, (payload.length() + 1));
    client.publish(PUB_TOPIC_2, data);
    last_pub = millis();
  }
  client.loop();
}
