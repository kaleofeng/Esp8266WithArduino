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

const char httpsUrl[] PROGMEM = "https://www.metazion.net/print";
const char httpsFingerprint[] PROGMEM = "5a fa b7 1d 1f 67 8d 88 df de d6 99 b5 6f b7 16 0a 7c 2c a9";

String realIp;
String realPort;

U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 0, /* data=*/ 2, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 16);

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
  WiFiMulti.addAP("YJKJ-JSB", "012345678");
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

      JSONVar jsonObject = JSON.parse(payload.c_str());
      JSONVar keys = jsonObject.keys();
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = jsonObject[keys[i]];
        Serial.printf("[HTTPS] Rsp data: %s %s\n", (const char*)keys[i], JSONVar::stringify(value).c_str());
      }

      if (jsonObject.hasOwnProperty("realIp")) {
        realIp = jsonObject["realIp"];
      }
      
      if (jsonObject.hasOwnProperty("realPort")) {
        realPort = jsonObject["realPort"];
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
