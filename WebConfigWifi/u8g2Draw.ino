
void u8g2DrawSetup() {
  u8g2.begin();
}

bool u8g2DrawLoop() {
  u8g2.firstPage();  
  do {
    u8gDrawSomething();
  } while(u8g2.nextPage());
}

void u8g2DrawPrepare() {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void u8gDrawSomething() {
  u8g2DrawPrepare();

  bool hasSSID = strlen(ssid) > 0;
  if (!hasSSID) {
    u8gDrawConfigWifi();
  }
  else if (status != WL_CONNECTED) {
    u8gDrawConnectWifi();
  }
  else {
    u8gDrawRealData();
  }
}

void u8gDrawConfigWifi() {
  u8g2.drawStr(0, 0, "Please Config Wifi");
  u8g2.drawStr(0, 20, "SSID:");
  u8g2.drawStr(48, 20, apSSID);
  u8g2.drawStr(0, 30, "PWD:");
  u8g2.drawStr(48, 30, apPassword);
  u8g2.drawStr(0, 40, "Http:");
  u8g2.drawStr(48, 40, apIP.toString().c_str());
}

void u8gDrawConnectWifi() {
  u8g2.drawStr(0, 0, "Connecting to Wifi");
  u8g2.drawStr(0, 20, "SSID:");
  u8g2.drawStr(48, 20, ssid);
  u8g2.drawStr(0, 30, "PWD:");
  u8g2.drawStr(48, 30, password);
  u8g2.drawStr(0, 40, "Status:");
  u8g2.drawStr(48, 40, String(status).c_str());
}

void u8gDrawRealData() {
  u8g2.drawStr(0, 0, "Response Data");
  u8g2.drawStr(0, 20, "IP:");
  u8g2.drawStr(36, 20, realIp.c_str());
  u8g2.drawStr(0, 30, "Port:");
  u8g2.drawStr(36, 30, realPort.c_str());
}
