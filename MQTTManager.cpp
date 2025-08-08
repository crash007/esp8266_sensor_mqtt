#include "MQTTManager.h"

#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <DallasTemperature.h> // For DeviceAddress (via Settings.h)
#include "Settings.h"
#include "WiFiManager.h"
#include "DeepSleep.h"
#include "Debug.h"

AsyncMqttClient client;
Ticker mqttReconnectTimer;
Ticker mqttTimeoutTimer;

// Struktur för ett MQTT-meddelande
struct MqttMessage {
  const char* topic;
  String payload;
};

#define MAX_MESSAGES 10
MqttMessage mqttMessages[MAX_MESSAGES];
int mqttMessageCount = 0;
int currentMessageIndex = 0;

// Globala variabler
float lastDs18b20Temp;
float lastBatteryVoltage;
unsigned long uploadStart;
extern unsigned long totalTimeStart;

void queueMqttMessage(const char* topic, float value) {
  if (mqttMessageCount < MAX_MESSAGES) {
    mqttMessages[mqttMessageCount++] = { topic, String(value, 3) };
  }
}

void publishNextMessage() {
  if (currentMessageIndex < mqttMessageCount) {
    MqttMessage& msg = mqttMessages[currentMessageIndex];
    client.publish(msg.topic, 1, false, msg.payload.c_str());
    currentMessageIndex++;
  } else {
    DEBUG_PRINTLN("All MQTT messages published");
    client.disconnect();

    unsigned long uploadTime = millis() - uploadStart;
    DEBUG_PRINT("Upload took: "); DEBUG_PRINTLN(uploadTime);
    DEBUG_PRINT("Total time: "); DEBUG_PRINTLN(millis() - totalTimeStart);
    DEBUG_PRINTLN("");

    saveApChannelBssid();
    deepSleep();
  }
}

void onPublish(uint16_t packetId) {
  DEBUG_PRINT("Published message "); DEBUG_PRINTLN(currentMessageIndex - 1);
  publishNextMessage();
}

void onMqttConnect(bool sessionPresent) {
  DEBUG_PRINTLN("Connected to MQTT.");
  mqttTimeoutTimer.detach();
  currentMessageIndex = 0;
  publishNextMessage();
}


void mqttTimeoutHandler() {
  DEBUG_PRINTLN("MQTT connection timeout. Forcing disconnect and deep sleep.");
  deepSleep();
}

void connectToMqtt() {
  DEBUG_PRINTLN("Connecting to MQTT...");
  client.connect();

   // Starta timeout (t.ex. 5 sekunder)
  mqttTimeoutTimer.once(5, mqttTimeoutHandler);
}


void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  DEBUG_PRINTLN("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  } else {
    deepSleep();
  }
}



void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum, float batteryVoltage) {
  mqttMessageCount = 0;

  queueMqttMessage(DS18B20_TOPIC, ds18b20Temp);
  queueMqttMessage(BATTERY_VOLTAGE_TOPIC, batteryVoltage);
 // queueMqttMessage(DHT_TEMP_TOPIC, dhtTemp);
 // queueMqttMessage(DHT_HUMIDITY_TOPIC, dhtHum);

  uploadStart = millis();
  

  client.onConnect(onMqttConnect);
  client.onDisconnect(onMqttDisconnect);
  client.onPublish(onPublish);
 

  DEBUG_PRINT("Setting MQTT server: "); DEBUG_PRINTLN(MQTT_SERVER);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  connectToMqtt();
}
