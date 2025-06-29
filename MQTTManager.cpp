#include "MQTTManager.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <DallasTemperature.h> // For DeviceAddress (via Settings.h)
#include "Settings.h"
#include "WiFiManager.h"
#include "DeepSleep.h"
#include "Debug.h"

WiFiClient espClient;
PubSubClient client(espClient);

void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum) {
  int retries = 0;

  unsigned long uploadStart = millis();
  client.setServer(MQTT_SERVER, MQTT_PORT);

  while (!client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
      DEBUG_PRINTLN("connected");
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINTLN(client.state());
      if (retries == 1) {
        wifiReconnectCachedBssid();
      }
      if (retries == MQTT_MAX_RETRIES) {
        DEBUG_PRINTLN("Unable to connect");
        deepSleep();
      }
      delay(MQTT_CONNECT_DELAY);
    }
    retries++;
  }

  char buff[8];
  snprintf(buff, sizeof(buff), "%.3f", ds18b20Temp);
  client.publish(DS18B20_TOPIC, buff);
  delay(MQTT_PUBLISH_DELAY);

#ifdef ESP8266
  //snprintf(buff, sizeof(buff), "%.3f", dhtTemp);
  //client.publish("/v1/esp8266/dht22-temp", buff);
  //delay(MQTT_PUBLISH_DELAY);
  //snprintf(buff, sizeof(buff), "%.3f", dhtHum);
  //client.publish("/v1/esp8266/dht22-hum", buff);
  //delay(MQTT_PUBLISH_DELAY);
#else
  //snprintf(buff, sizeof(buff), "%.3f", dhtTemp);
  //client.publish("/v1/esp32/dht22-temp", buff);
  //delay(MQTT_PUBLISH_DELAY);
  //snprintf(buff, sizeof(buff), "%.3f", dhtHum);
  //client.publish("/v1/esp32/dht22-hum", buff);
  //delay(MQTT_PUBLISH_DELAY);
#endif

  DEBUG_PRINTLN("Publish complete");
  delay(MQTT_DISCONNECT_DELAY);
  client.disconnect();
  unsigned long uploadTime = millis() - uploadStart;
  DEBUG_PRINT("Upload took: ");
  DEBUG_PRINTLN(uploadTime);
}
