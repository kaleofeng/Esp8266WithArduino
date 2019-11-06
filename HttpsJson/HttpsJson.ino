/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>
#include <Arduino_JSON.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

ESP8266WiFiMulti WiFiMulti;
const char wifiSSID[] = "Neptune";
const char wifiPassword[] = "1234509876";

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

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiSetup();
}

void loop() {
  fetchDataLoop();

  Serial.println("Wait 10s before next round...");
  delay(10000);
}

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(wifiSSID, wifiPassword);
}

bool fetchDataLoop() {
  Serial.printf("[HTTPS] Connect wifi(%s)...\n", wifiSSID);
  
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.println("[HTTPS] WiFi connect failed");
    return false;
  }

  Serial.printf("[HTTPS] Set fingerprint(%s)...\n", httpsFingerprint);
  
  BearSSL::WiFiClientSecure client;
  bool ret = client.setFingerprint(httpsFingerprint);
  if (!ret) {
    Serial.println("[HTTPS] Set fingerprint failed");
    return false;
  }

  Serial.printf("[HTTPS] Connect remote(%s)...\n", httpsUrl);
  
  HTTPClient https;
  ret = https.begin(client, httpsUrl);
  if (!ret) {
    Serial.println("[HTTPS] Begin connec failed");
    return false;
  }
  
  Serial.println("[HTTPS] Req data...");

  int httpCode = https.GET();
  
  Serial.printf("[HTTPS] Rsp code: %d %s\n", httpCode, https.errorToString(httpCode).c_str());
    
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();
      Serial.printf("[HTTPS] Rsp payload: %s\n", payload.c_str());

      JSONVar jsonObject = JSON.parse(payload.c_str());
      JSONVar keys = jsonObject.keys();
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = jsonObject[keys[i]];
        Serial.printf("[HTTPS] Rsp data: %s %s\n", (const char*)keys[i], JSONVar::stringify(value).c_str());
      }

      if (jsonObject.hasOwnProperty("ip")) {
        realIp = jsonObject["ip"];
      }
      
      if (jsonObject.hasOwnProperty("port")) {
        realPort = (int)jsonObject["port"];
      }

      Serial.printf("[HTTPS] Got data: realIp(%s) realPort(%s)\n", realIp.c_str(), realPort.c_str());
    }
  }

  https.end();
  return true;
}
