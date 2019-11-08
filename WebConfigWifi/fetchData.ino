
bool fetchDataLoop() {
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
