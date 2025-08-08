#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "Settings.h"
//#include <PubSubClient.h>

//extern WiFiClient espClient;
//extern PubSubClient client;

void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum, float batteryVoltage);

#endif
