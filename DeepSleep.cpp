#include "DeepSleep.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <DallasTemperature.h> // For DeviceAddress (via Settings.h)
#include "Settings.h"
#include "Debug.h"
#include "rtc.h"

void deepSleep() {
  DEBUG_PRINTLN("Going to sleep.");
  //WiFi.disconnect(true);
  //delay(DEEP_SLEEP_DELAY);
  //WiFi.mode(WIFI_OFF);
  //WiFi.forceSleepBegin(); // ESP8266 only
  //delay(5);

#ifdef DEBUG
  Serial.flush();
  //Serial.end();
#endif

  WiFi.disconnect(true);
  delay(DEEP_SLEEP_DELAY);
  WiFi.mode(WIFI_OFF);
  delay(DEEP_SLEEP_DELAY);
#ifdef ESP8266
  WiFi.forceSleepBegin();
  delay(DEEP_SLEEP_DELAY);
  ESP.deepSleep(SLEEP_TIME, WAKE_RF_DEFAULT);
#else
  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  esp_deep_sleep_start();
#endif
}

void saveApChannelBssid() {
  rtcData.channel = WiFi.channel();
  memcpy(rtcData.ap_mac, WiFi.BSSID(), 6);
  rtcData.ip = WiFi.localIP();
  rtcData.subnet = WiFi.subnetMask();
  rtcData.gateway = WiFi.gatewayIP();
  rtcData.dns = WiFi.dnsIP();
  rtcData.crc32 = calculateCRC32(((uint8_t*)(&rtcData)) + 4, sizeof(rtcData) - 4);
#ifdef ESP8266
  ESP.rtcUserMemoryWrite(0, (uint32_t*)(&rtcData), sizeof(rtcData));
#endif
}
