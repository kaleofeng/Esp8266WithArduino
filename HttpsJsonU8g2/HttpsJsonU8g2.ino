/**
  HttpsJsonU8g2.ino

  Created on: 07.11.2019

*/

#include <Arduino.h>
#include <Arduino_JSON.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

ESP8266WiFiMulti WiFiMulti;
const char wifiSSID[] = "Neptune";
const char wifiPassword[] = "1234509876";

const char httpsUrl[] PROGMEM = "https://ifconfig.co/port/8080";
const char httpsFingerprint[] PROGMEM = "98 55 01 22 54 27 b0 22 7c 7b de 33 2e 9e 04 3a 09 51 b0 24";

String realIp;
String realPort;

U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* cs=*/ 15, /* dc=*/ 12, /* reset=*/ 13);

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
  u8g2Setup();
}

void loop() {
  fetchDataLoop();
  drawSomethingLoop();

  Serial.println("Wait 10s before next round...");
  delay(10000);
}

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(wifiSSID, wifiPassword);
}

void u8g2Setup() {
  u8g2.begin();
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

void u8g2Prepare() {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void drawSomething() {
  u8g2Prepare();

  u8g2.drawStr(0, 0, realIp.c_str());
  u8g2.drawStr(0, 20, realPort.c_str());
}

bool drawSomethingLoop() {
  u8g2.firstPage();  
  do {
    drawSomething();
  } while(u8g2.nextPage());
}
