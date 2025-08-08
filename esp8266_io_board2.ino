#include "Sensor.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DeepSleep.h"
#include "Debug.h"
#include "rtc.h"


unsigned long totalTimeStart;
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
 // WiFi.printDiag(Serial);
#endif
 totalTimeStart = millis();
logWakeUpReason();


// Invalidate RTC data on power reset
#ifdef ESP32
if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
  DEBUG_PRINTLN("Power reset detected, invalidating RTC data");
  invalidateRtcData();
} else {
  DEBUG_PRINT("Wakeup cause: ");
  switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_TIMER: DEBUG_PRINTLN("Timer"); break;
    default: DEBUG_PRINTLN("Other"); break;
  }
}
#else // ESP8266
if (!isRtcValid()) {
  DEBUG_PRINTLN("ESP8266: Invalid RTC data, assuming power reset");
  invalidateRtcData();
}
#endif

 
  DEBUG_PRINT("Time before wifiBegin: ");
  DEBUG_PRINTLN( millis()-totalTimeStart);
  unsigned long wifiConnectStart = wifiBegin();

//  if (!initBME280()) {
//    DEBUG_PRINTLN("BME280 init failed");
//  }


 float bat = readBatteryVoltage();
  float ds18b20Temp = readDS18B20();
  //float bmeTemp = readBmeTemperature();
  //float bmeHum = readBmeHumidity();
  //float bmePress = readBmePressure();
  
  
  float dhtTemp = 0.0; //readDhtTemp(); // Commented in original
  float dhtHum = 0.0; //readDhtHumidity(); // Commented in original

  wifiWaitConnected();
  printWifiInfo(wifiConnectStart);

  mqttUpload(ds18b20Temp, dhtTemp, dhtHum, bat);

}
