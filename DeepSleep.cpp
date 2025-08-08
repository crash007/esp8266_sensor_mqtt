#include "DeepSleep.h"

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

void logWakeUpReason() {
  
#ifdef ESP8266
  DEBUG_PRINT("Reset reason: ");
  DEBUG_PRINTLN(ESP.getResetReason());
#else // ESP32
  DEBUG_PRINT("Reset reason: ");
  switch (esp_reset_reason()) {
    case ESP_RST_UNKNOWN: DEBUG_PRINTLN("Unknown"); break;
    case ESP_RST_POWERON: DEBUG_PRINTLN("Power-on"); break;
    case ESP_RST_EXT: DEBUG_PRINTLN("External"); break;
    case ESP_RST_SW: DEBUG_PRINTLN("Software"); break;
    case ESP_RST_PANIC: DEBUG_PRINTLN("Panic"); break;
    case ESP_RST_INT_WDT: DEBUG_PRINTLN("Interrupt Watchdog"); break;
    case ESP_RST_TASK_WDT: DEBUG_PRINTLN("Task Watchdog"); break;
    case ESP_RST_WDT: DEBUG_PRINTLN("Other Watchdog"); break;
    case ESP_RST_DEEPSLEEP: DEBUG_PRINTLN("Deep Sleep"); break;
    case ESP_RST_BROWNOUT: DEBUG_PRINTLN("Brownout"); break;
    case ESP_RST_SDIO: DEBUG_PRINTLN("SDIO"); break;
    default: DEBUG_PRINTLN("Unknown reset reason"); break;
  }
#endif
}
