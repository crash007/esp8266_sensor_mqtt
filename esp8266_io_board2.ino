#include "Sensor.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DeepSleep.h"
#include "Debug.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

void setup() {
  run();
}

void loop() {
  // Empty, as the device uses deep sleep
}

void run() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  while (!Serial);
  DEBUG_PRINTLN("setup");
  DEBUG_PRINT("Wakeup reason: ");
  DEBUG_PRINTLN(ESP.getResetReason());
  WiFi.printDiag(Serial);
#endif

  unsigned long totalTimeStart = millis();
  unsigned long wifiConnectStart = wifiBegin();

  //WiFi.mode(WIFI_OFF);
  //WiFi.forceSleepBegin();
  //delay(1);
  //WiFi.setAutoConnect(false);

  float ds18b20Temp = readDS18B20();
  float dhtTemp = 0.0; //readDhtTemp(); // Commented in original
  float dhtHum = 0.0; //readDhtHumidity(); // Commented in original

  wifiWaitConnected();
  printWifiInfo(wifiConnectStart);

  mqttUpload(ds18b20Temp, dhtTemp, dhtHum);
  saveApChannelBssid();

  DEBUG_PRINT("Total time:");
  DEBUG_PRINTLN(millis() - totalTimeStart);
  DEBUG_PRINTLN("");
  deepSleep();
}
