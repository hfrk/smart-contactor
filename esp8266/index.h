#ifndef _ESP_INDEX_H_
#define _ESP_INDEX_H_

#include "Arduino.h"

const char response_state_true[] PROGMEM = R"state_true(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 WiFi AP</title>
  <style>
body {
  max-width: 300px;
  margin: 0 auto;
  font-family: 'Open Sans', sans-serif;
  font-size: 16px;
  line-height: 1.6em;
  box-sizing: border-box;
  text-align: center;
}
h1 {
  margin-top: 20px;
  margin-bottom: 10px;
  font-size: 2.5em;
  font-weight: normal;
}
h2 {
  margin: 0px;
  font-size: 20px;
  font-weight: normal;
}
h3 {
  margin-top: 20px;
  margin-bottom: 10px;
  font-size: 1.6em;
  font-weight: normal;
}
p {
  color: #c19e9e;
}
.wifi-settings {
  border: 1px solid green;
  max-width: 256px;
  padding: 10px;
}
.btn-submit {
  background-color: black;
  border: 1px solid grey;
  border-radius: 2px;
  margin: 10px;
}
.button {
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  display: inline-block;
  font-size: 16px;
  margin: 20px;
  cursor: pointer;
}
.red {color: white; background-color: red;}
.green {color: white; background-color: green;}
.switch {
  display: none;
  position: relative;
  margin: 0 10px;
  font-size: 16px;
  line-height: 24px;
}
.switch__input {
  position: absolute;
  top: 0;
  left: 0;
  width: 36px;
  height: 20px;
  opacity: 0;
  z-index: 0;
}
.switch__label {
  display: block;
  padding: 0 0 0 44px;
  cursor: pointer;
}
.switch__label:before {
  content: "";
  position: absolute;
  top: 5px;
  left: 0;
  width: 36px;
  height: 14px;
  background-color: #6f6f6f80;
  border-radius: 14px;
  z-index: 1;
  transition: background-color 0.28s cubic-bezier(0.4, 0, 0.2, 1);
}
.switch__label:after {
  content: "";
  position: absolute;
  top: 2px;
  left: 0;
  width: 20px;
  height: 20px;
  background-color: #949494;
  border-radius: 14px;
  box-shadow: 0 2px 2px 0 rgba(0, 0, 0, 0.14), 0 3px 1px -2px rgba(0, 0, 0, 0.2), 0 1px 5px 0 rgba(0, 0, 0, 0.12);
  z-index: 2;
  transition: all 0.28s cubic-bezier(0.4, 0, 0.2, 1);
  transition-property: left, background-color;
}
.switch__input:checked + .switch__label:before {
  background-color: #20c20080;
}
.switch__input:checked + .switch__label:after {
  left: 16px;
  background-color: #20c200;
}
  </style>
</head>
<body>
  <div class="title">
    <h1>ESP32 WiFi AP</h1>
  </div>
  <div class="contactor">
    <div style="display: inline-flex; text-align: center;">
      <h2 style="margin: 0 auto;">Contactor:</h2>
      <div class="switch" id="hide-noscript">
        <input type="checkbox" id="contactor" class="switch__input" onclick="window.location = '/off'" checked>
        <label for="contactor" class="switch__label"></label>
      </div>
      <h2 style="color: green">on</h2>
      <noscript>
        <a href="/off" class="red">Matikan</a>
      </noscript>
    </div>
  </div>
  <div class="wifi-settings">
    <h2>Atur koneksi ke WiFi dan MQTT Node-RED:</h2>
    <form action="/wifi-settings" method="post">
      <label for="ssid">WiFi SSID:</label><br>
      <input type="text" id="ssid" name="ssid" placeholder="Isikan SSID WiFi..." required><br>
      <label for="pass">WiFi Password:</label><br>
      <input type="password" id="pass" name="pass" placeholder="(kosongkan jika WiFi open)"><br>
      <label for="mqtt">MQTT Server:</label><br>
      <input type="text" id="mqtt" name="mqtt" placeholder="192.168.xxx.xxx:port"><br>
      <input type="submit" class="button btn-submit" value="Submit">
    </form>
  </div>
  <script type="text/javascript">document.getElementById('hide-noscript').style.display='inline-block';</script>
</body>
</html>
)state_true";

const char response_state_false[] PROGMEM = R"state_false(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 WiFi AP</title>
  <style>
body {
  max-width: 300px;
  margin: 0 auto;
  font-family: 'Open Sans', sans-serif;
  font-size: 16px;
  line-height: 1.6em;
  box-sizing: border-box;
  text-align: center;
}
h1 {
  margin-top: 20px;
  margin-bottom: 10px;
  font-size: 2.5em;
  font-weight: normal;
}
h2 {
  margin: 0px;
  font-size: 20px;
  font-weight: normal;
}
h3 {
  margin-top: 20px;
  margin-bottom: 10px;
  font-size: 1.6em;
  font-weight: normal;
}
p {
  color: #c19e9e;
}
.wifi-settings {
  border: 1px solid green;
  max-width: 256px;
  padding: 10px;
}
.btn-submit {
  background-color: black;
  border: 1px solid grey;
  border-radius: 2px;
  margin: 10px;
}
.button {
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  display: inline-block;
  font-size: 16px;
  margin: 20px;
  cursor: pointer;
}
.red {color: white; background-color: red;}
.green {color: white; background-color: green;}
.switch {
  display: none;
  position: relative;
  margin: 0 10px;
  font-size: 16px;
  line-height: 24px;
}
.switch__input {
  position: absolute;
  top: 0;
  left: 0;
  width: 36px;
  height: 20px;
  opacity: 0;
  z-index: 0;
}
.switch__label {
  display: block;
  padding: 0 0 0 44px;
  cursor: pointer;
}
.switch__label:before {
  content: "";
  position: absolute;
  top: 5px;
  left: 0;
  width: 36px;
  height: 14px;
  background-color: #6f6f6f80;
  border-radius: 14px;
  z-index: 1;
  transition: background-color 0.28s cubic-bezier(0.4, 0, 0.2, 1);
}
.switch__label:after {
  content: "";
  position: absolute;
  top: 2px;
  left: 0;
  width: 20px;
  height: 20px;
  background-color: #949494;
  border-radius: 14px;
  box-shadow: 0 2px 2px 0 rgba(0, 0, 0, 0.14), 0 3px 1px -2px rgba(0, 0, 0, 0.2), 0 1px 5px 0 rgba(0, 0, 0, 0.12);
  z-index: 2;
  transition: all 0.28s cubic-bezier(0.4, 0, 0.2, 1);
  transition-property: left, background-color;
}
.switch__input:checked + .switch__label:before {
  background-color: #20c20080;
}
.switch__input:checked + .switch__label:after {
  left: 16px;
  background-color: #20c200;
}
  </style>
</head>
<body>
  <div class="title">
    <h1>ESP32 WiFi AP</h1>
  </div>
  <div class="contactor">
    <div style="display: inline-flex; text-align: center;">
      <h2 style="margin: 0 auto;">Contactor:</h2>
      <div class="switch" id="hide-noscript">
        <input type="checkbox" id="contactor" class="switch__input" onclick="window.location = '/on'">
        <label for="contactor" class="switch__label"></label>
      </div>
      <h2 style="color: red">off</h2>
      <noscript>
        <a href="/on" class="green">Nyalakan</a>
      </noscript>
    </div>
  </div>
  <div class="wifi-settings">
    <h2>Atur koneksi ke WiFi dan MQTT Node-RED:</h2>
    <form action="/wifi-settings" method="post">
      <label for="ssid">WiFi SSID:</label><br>
      <input type="text" id="ssid" name="ssid" placeholder="Isikan SSID WiFi..." required><br>
      <label for="pass">WiFi Password:</label><br>
      <input type="password" id="pass" name="pass" placeholder="(kosongkan jika WiFi open)"><br>
      <label for="mqtt">MQTT Server:</label><br>
      <input type="text" id="mqtt" name="mqtt" placeholder="192.168.xxx.xxx:port"><br>
      <input type="submit" class="button btn-submit" value="Submit">
    </form>
  </div>
  <script type="text/javascript">document.getElementById('hide-noscript').style.display='inline-block';</script>
</body>
</html>
)state_false";

class IndexResponse {
public:
  IndexResponse() {};
  const char* getResponse(bool state) { return state ? response_state_true : response_state_false; }
};

#endif 
