
#include <stdio.h>
#include <string.h>
#include <PubSubClient.h>
#include "sign_api.h"

#define EXAMPLE_PRODUCT_KEY     "a1ePUXC9DUp"
#define EXAMPLE_PRODUCT_SECRET  "wW5ifNC9IE2bLYlL"
#define EXAMPLE_DEVICE_NAME     "Esp8266"
#define EXAMPLE_DEVICE_SECRET   "kds31295rw0EP3GxjryySS0lAqMsm2K1"

iotx_sign_mqtt_t sign_mqtt;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker mqttChecker;

void mqttCommSetup() {
  mqttCommGenerateCredentials();
  mqttClient.setServer(sign_mqtt.hostname, sign_mqtt.port);
  mqttChecker.attach(5, mqttCommCheckConnection);
}

void mqttCommLoop() {  
  mqttClient.loop();
}

void mqttCommGenerateCredentials() {
  iotx_dev_meta_info_t meta_info;
  memset(&meta_info, 0, sizeof(iotx_dev_meta_info_t));
  memcpy(meta_info.product_key, EXAMPLE_PRODUCT_KEY, strlen(EXAMPLE_PRODUCT_KEY));
  memcpy(meta_info.product_secret, EXAMPLE_PRODUCT_SECRET, strlen(EXAMPLE_PRODUCT_SECRET));
  memcpy(meta_info.device_name, EXAMPLE_DEVICE_NAME, strlen(EXAMPLE_DEVICE_NAME));
  memcpy(meta_info.device_secret, EXAMPLE_DEVICE_SECRET, strlen(EXAMPLE_DEVICE_SECRET));
  IOT_Sign_MQTT(IOTX_CLOUD_REGION_SHANGHAI, &meta_info, &sign_mqtt);
  Serial.printf("[MQTT] generate credentials hostname(%s) port(%d) clientid(%s) username(%s) password(%s)\n",
    sign_mqtt.hostname, sign_mqtt.port, sign_mqtt.clientid, sign_mqtt.username, sign_mqtt.password);
}

void mqttCommCheckConnection() {
  if (isConnected() && !mqttClient.connected()) {
    mqttCommReconnectServer();
  }
}

void mqttCommReconnectServer() {
  Serial.printf("[MQTT] reconnect server hostname(%s) port(%d) clientid(%s) username(%s) password(%s)\n",
    sign_mqtt.hostname, sign_mqtt.port, sign_mqtt.clientid, sign_mqtt.username, sign_mqtt.password);
    
  if (mqttClient.connect(sign_mqtt.clientid, sign_mqtt.username, sign_mqtt.password)) {   
    Serial.printf("[MQTT] connect success\n");
    return;
  }

  int errCode = mqttClient.state();
  Serial.printf("[MQTT] connect failed, errCode(%d)\n", errCode);
}
