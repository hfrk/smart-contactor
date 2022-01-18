#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include "index.h"

//#define MQTT_DEBUG

#define PUB_INTERVAL 100
#define MQTT_CONNECT_ATTEMPT_INTERVAL 5000
unsigned long last_pub = 0;
unsigned long last_mqtt_connect_attempt = 0;

// timer stuff
unsigned long timer_last;
unsigned long TIMER_SETTIME;
#define MINUTE 60000
bool isTimerOn = false;

const int RELAY_PIN = 2; // Relay
#define RELAY_PIN_ACTIVE LOW
bool state = false;
bool start_STA_flag = false;
String STASSID;
String STAPSK;
String MQTT;
int port;

// Ganti dengan ssid dan password wifi
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const int mqtt_port = 1883;
uint8_t mqtt_connect_tries = 0;

const char* ID = "ESP1";
// Subscribe topics:
const char* SUB_command = "command/ESP1";
const char* SUB_timer = "timer/ESP1";
const char* SUB_global_timer = "TIMER";
// Publish to topic:
const char* PUB_status = "status/ESP1";

char data[80];
bool immediate_update = false;
WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);
IndexResponse response;

void mqtt_connect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(ID)) {
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
  start_softAP();
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
      && WiFi.getMode() == WIFI_STA
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

  if (start_STA_flag) {
    start_STA(STASSID.c_str(), STAPSK.c_str(), MQTT.c_str(), port);
    start_STA_flag = false;
  }
  client.loop();
  server.handleClient();  
}

const char* createResponseFromState() {
/*
  String response = "<!DOCTYPE html><html><head> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>ESP32 WiFi AP</title> <style>body{max-width: 300px; margin: 0 auto; font-family: 'Open Sans', sans-serif; font-size: 16px; line-height: 1.6em; box-sizing: border-box; text-align: center;}h1{margin-top: 20px; margin-bottom: 10px; font-size: 2.5em; font-weight: normal;}h2{margin: 0px; font-size: 20px; font-weight: normal;}h3{margin-top: 20px; margin-bottom: 10px; font-size: 1.6em; font-weight: normal;}p{color: #c19e9e;}.wifi-settings{border: 1px solid green; max-width: 256px; padding: 10px;}.btn-submit{background-color: black; border: 1px solid grey; border-radius: 2px; margin: 10px;}.button{border: none; color: white; padding: 15px 32px; text-align: center; display: inline-block; font-size: 16px; margin: 20px; cursor: pointer;}.red{color: white; background-color: red;}.green{color: white; background-color: green;}.switch{display: none; position: relative; margin: 0 10px; font-size: 16px; line-height: 24px;}.switch__input{position: absolute; top: 0; left: 0; width: 36px; height: 20px; opacity: 0; z-index: 0;}.switch__label{display: block; padding: 0 0 0 44px; cursor: pointer;}.switch__label:before{content: \"\"; position: absolute; top: 5px; left: 0; width: 36px; height: 14px; background-color: #6f6f6f80; border-radius: 14px; z-index: 1; transition: background-color 0.28s cubic-bezier(0.4, 0, 0.2, 1);}.switch__label:after{content: \"\"; position: absolute; top: 2px; left: 0; width: 20px; height: 20px; background-color: #949494; border-radius: 14px; box-shadow: 0 2px 2px 0 rgba(0, 0, 0, 0.14), 0 3px 1px -2px rgba(0, 0, 0, 0.2), 0 1px 5px 0 rgba(0, 0, 0, 0.12); z-index: 2; transition: all 0.28s cubic-bezier(0.4, 0, 0.2, 1); transition-property: left, background-color;}.switch__input:checked + .switch__label:before{background-color: #20c20080;}.switch__input:checked + .switch__label:after{left: 16px; background-color: #20c200;}</style></head><body> <div class=\"title\"> <h1>ESP32 WiFi AP</h1> </div><div class=\"contactor\"> <div style=\"display: inline-flex; text-align: center;\"> <h2 style=\"margin: 0 auto;\">Contactor:</h2> <div class=\"switch\" id=\"hide-noscript\">";
  response += (state) ? " <input type=\"checkbox\" id=\"contactor\" class=\"switch__input\" onclick=\"window.location='/off'\" checked>"
                      : " <input type=\"checkbox\" id=\"contactor\" class=\"switch__input\" onclick=\"window.location='/on'\">";
  response += " <label for=\"contactor\" class=\"switch__label\"></label> </div>";
  response += (state) ? "<h2 style=\"color: green\">on</h2>"
                      : "<h2 style=\"color: red\">off</h2>";
  response += "<br> <noscript>";
  response += (state) ? " <a href=\"/off\" class=\"red\">Matikan</a>"
                      : " <a href=\"/on\" class=\"green\">Nyalakan</a>";
  response += " </noscript> </div></div><div class=\"wifi-settings\"> <h2>Atur koneksi ke WiFi dan MQTT Node-RED:</h2> <form action=\"/wifi-settings\" method=\"post\"> <label for=\"ssid\">WiFi SSID:</label><br><input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"Isikan SSID WiFi...\" required><br><label for=\"pass\">WiFi Password:</label><br><input type=\"password\" id=\"pass\" name=\"pass\" placeholder=\"(kosongkan jika WiFi open)\"><br><label for=\"mqtt\">MQTT Server:</label><br><input type=\"text\" id=\"mqtt\" name=\"mqtt\" placeholder=\"192.168.xxx.xxx:port\"><br><input type=\"submit\" class=\"button btn-submit\" value=\"Submit\"> </form> </div><script type=\"text/javascript\">document.getElementById('hide-noscript').style.display='inline-block';</script></body></html>";
*/
  return response.getResponse(state);
}

void handleRoot() {
  server.send_P(200, "text/html", createResponseFromState());
}

void start_softAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ID);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/on", []() {
    state = true;
    server.send_P(200, "text/html", createResponseFromState());
  });
  server.on("/off", []() {
    state = false;
    server.send_P(200, "text/html", createResponseFromState());
  });
  server.on("/wifi-settings", HTTP_POST, []() {
    String message;
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      if (strncmp(server.argName(i).c_str(), "ssid", 4) == 0) {
        STASSID = String(server.arg(i));
      } else if (strncmp(server.argName(i).c_str(), "pass", 4) == 0) {
        STAPSK = String(server.arg(i));
      } else if (strncmp(server.argName(i).c_str(), "mqtt", 4) == 0) {
        MQTT = server.arg(i).substring(0, server.arg(i).indexOf(':'));
        port = server.arg(i).substring(server.arg(i).indexOf(':')+1).toInt();
      }
    }
    Serial.println(message);
    Serial.println(server.argName(0));
    Serial.println(server.arg(0));
    server.send(200, "text/plain", "POST OK!\r\n" + message);
    start_STA_flag = true;
  });
  server.begin();
  Serial.println("HTTP server started");
}

void start_STA(const char* STASSID, const char* STAPSK, const char* MQTT, int port) {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(STASSID, STAPSK);
  Serial.printf("Connecting to %s", STASSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  client.setServer(MQTT, port);
  client.setCallback(mqtt_callback);
  delay(2000);
}
