/**
  WebConfigWifi.ino

  Created on: 08.11.2019

*/

#include <Arduino.h>
#include <Arduino_JSON.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Ticker.h>

const int STAGE_CONNECTING = 0;
const int STAGE_CONNECTED = 1;
const int STAGE_COMMUNICATING = 2;

IPAddress apIP(192, 168, 4, 1);
IPAddress apNetMask(255, 255, 255, 0);
const char apSSID[] = "XiHuan";
const char apPassword[] = "1234509876";
const char myHostname[] = "esp8266";
ESP8266WebServer server(80);

char ssid[32] = "";
char password[32] = "";

unsigned int status = WL_IDLE_STATUS;
boolean connect = false;
IPAddress localIP;
int stage = 0;
const unsigned long stageDuration = 3000; // ms
unsigned long stageTime = 0; // ms

const unsigned long reconnectInterval = 10000; // ms
unsigned long lastReconnectTime = 0; // ms

const unsigned long logicInterval = 10000; // ms
unsigned long lastLogicTime = 0; // ms

const unsigned long renderInterval = 100; // ms
unsigned long lastRenderTime = 0; // ms

const char httpsUrl[] PROGMEM = "https://ifconfig.co/port/8080";
const char httpsFingerprint[] PROGMEM = "98 55 01 22 54 27 b0 22 7c 7b de 33 2e 9e 04 3a 09 51 b0 24";

String realIp;
String realPort;

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 3; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiSetup();
  mqttCommSetup();
  fetchDataSetup();
  u8g2DrawSetup();
}

void loop() {
  wifiLoop();
  mqttCommLoop();
  runningLoop();
}

void wifiSetup() {
  WiFi.softAPConfig(apIP, apIP, apNetMask);
  WiFi.softAP(apSSID, apPassword);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("[WIFI] AP ip address: ");
  Serial.println(myIP);
  
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[WIFI] HTTP server started");

  loadCredentials();
  connect = strlen(ssid) > 0;
}

void wifiLoop() {
  server.handleClient();
  
  if (connect) {
    connect = false;
    localIP = IPAddress();
    stage = 0;
    stageTime = 0;
    lastReconnectTime = millis();
    
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int result = WiFi.waitForConnectResult();
    Serial.printf("[WIFI] Connect to ssid(%s) with password(%s) result(%d)\n", ssid, password, result);
  }
  
  unsigned int s = WiFi.status();
  if (WL_IDLE_STATUS == s && millis() >= (lastReconnectTime + reconnectInterval)) {
    connect = true;
  }
  
  if (status != s) {
    Serial.printf("[WIFI] Status changed from %d to %d\n", status, s);
    status = s;

    if (WL_CONNECTED == status) {
      localIP = WiFi.localIP();
    }
  }
}

void runningLoop() {
  const unsigned long now = millis();
  switch (status) {
    case WL_CONNECTED:
      logicLoop(now);
      break;
    case WL_NO_SSID_AVAIL:
      WiFi.disconnect();
      break;
    default:
      break;
  }

  renderLoop(now);
}

void logicLoop(unsigned long now) {
  if (now >= (lastLogicTime + logicInterval)) {
    lastLogicTime = now;
    
    fetchDataLoop();
    Serial.printf("[RUNNING] Wait %d ms before next logic round...\n", logicInterval);
  }
}

void renderLoop(unsigned long now) {
  if (now >= (lastRenderTime + renderInterval)) {
    lastRenderTime = now;
    u8g2DrawLoop();
  }
}

void checkAndSwitchStage() {
  if (stageTime == 0) {
    stageTime = millis();
  }
  
  if (millis() >= stageTime + stageDuration) {
    const int newStage = stage + 1;
    if (checkStageCanIn(newStage)) {
      stage = newStage;
      stageTime = 0;
    }
  }
}

bool checkStageCanIn(int stage) {
  switch (stage) {
    case STAGE_CONNECTED:
      return WL_CONNECTED == status;
    default:
      return true;
  }
}

bool isConnected() {
  return WL_CONNECTED == status;
}
