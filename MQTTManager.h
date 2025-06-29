#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <PubSubClient.h>

extern WiFiClient espClient;
extern PubSubClient client;

void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum);

#endif
