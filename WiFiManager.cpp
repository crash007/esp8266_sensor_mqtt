#include "WiFiManager.h"
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <DallasTemperature.h> // For DeviceAddress (via Settings.h)
#include "Settings.h"
#include "DeepSleep.h"
#include "Debug.h"
#include "rtc.h"

unsigned long wifiBegin() {
  unsigned long wifiConnectStart = millis();
#ifdef ESP8266
  //WiFi.printDiag(Serial);
  WiFi.forceSleepWake();
  delay(WIFI_RECONNECT_DELAY);
  //WiFi.printDiag(Serial);
#endif
  WiFi.persistent(false);
  delay(DEEP_SLEEP_DELAY);
  WiFi.mode(WIFI_STA);
  //delay(WIFI_RECONNECT_DELAY);
  //WiFi.printDiag(Serial);

  if (isRtcValid()) {
    DEBUG_PRINTLN("RTC is valid.");
    WiFi.config(rtcData.ip, rtcData.gateway, rtcData.subnet, rtcData.dns);
    WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
  } else {
    DEBUG_PRINTLN("RTC is not valid.");
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.begin(SSID, WIFI_PASSWORD);
  }
  return wifiConnectStart;
}

void wifiWaitConnected() {
  int retries = 0;

  while (WiFi.status() != WL_CONNECTED) {
    retries++;
    switch (WiFi.status()) {
      case WL_NO_SHIELD: DEBUG_PRINTLN("No WiFi shield is present"); break;
      case WL_IDLE_STATUS: DEBUG_PRINTLN("WiFi is in idle status"); break;
      case WL_NO_SSID_AVAIL: DEBUG_PRINTLN("No SSID available - check your SSID"); break;
      case WL_SCAN_COMPLETED: DEBUG_PRINTLN("WiFi scan completed"); break;
      case WL_CONNECTION_LOST: DEBUG_PRINTLN("Connection lost"); break;
      case WL_DISCONNECTED: DEBUG_PRINT("WiFi is disconnected"); break;
      default: DEBUG_PRINTLN("Unknown status"); break;
    }

    if (WiFi.status() == WL_CONNECT_FAILED) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Please verify credentials: ");
      deepSleep();
    }

    DEBUG_PRINT(WiFi.status());
    DEBUG_PRINTLN("...");

    if (retries == WIFI_SSID_FAIL_RETRIES && WiFi.status() == WL_NO_SSID_AVAIL) {
      DEBUG_PRINTLN("NO SSID available going to sleep");
      deepSleep();
    }

    if (isRtcValid() && retries == WIFI_QUICK_CONNECT_RETRIES) {
      DEBUG_PRINTLN("Quick connect not working. Trying new scan.");
      wifiReconnect();
    }

    if (retries == WIFI_RECONNECT_RETRIES) {
      wifiReconnect();
    }

#ifdef ESP8266
    //if (retries == 200) {
    //  DEBUG_PRINTLN("Disable wifi and wait 3 sek");
    //  WiFi.disconnect(true);
    //  delay(WIFI_RECONNECT_DELAY);
    //  WiFi.mode(WIFI_OFF);
    //  delay(WIFI_RECONNECT_DELAY);
    //  WiFi.forceSleepBegin();
    //  delay(3000);
    //  WiFi.forceSleepWake();
    //  delay(WIFI_RECONNECT_DELAY);
    //  WiFi.persistent(false);
    //  delay(WIFI_RECONNECT_DELAY);
    //  WiFi.mode(WIFI_STA);
    //  delay(WIFI_RECONNECT_DELAY);
    //  WiFi.begin(SSID, WIFI_PASSWORD);
    //}
#endif

    if (retries == WIFI_MAX_RETRIES) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
      deepSleep();
    }
    delay(WIFI_CONNECT_DELAY);
  }
}

void wifiReconnectCachedBssid() {
  WiFi.disconnect();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
#ifdef ESP8266
  WiFi.forceSleepBegin();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
  WiFi.forceSleepWake();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
#endif
  WiFi.persistent(false);

  if (isRtcValid()) {
    WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
  } else {
    WiFi.config(0U, 0U, 0U, 0U);
    WiFi.begin(SSID, WIFI_PASSWORD);
  }

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    retries++;
    if (WiFi.status() == WL_CONNECT_FAILED) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Please verify credentials: ");
      deepSleep();
    }
    DEBUG_PRINTLN("...");
    if (retries == WIFI_RECONNECT_RETRIES) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
      deepSleep();
    }
    delay(WIFI_RECONNECT_BSSID_DELAY);
  }

  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINT("Gateway address: ");
  DEBUG_PRINTLN(WiFi.gatewayIP());
  DEBUG_PRINT("DNS address: ");
  WiFi.dnsIP().printTo(Serial);
  DEBUG_PRINTLN("");
}

void wifiReconnect() {
  DEBUG_PRINTLN("Reconnecting......");
#ifdef ESP8266
  //WiFi.printDiag(Serial);
#endif
  WiFi.disconnect();
  delay(WIFI_RECONNECT_DELAY);
#ifdef ESP8266
  WiFi.forceSleepBegin();
  delay(WIFI_RECONNECT_DELAY);
  WiFi.forceSleepWake();
  delay(WIFI_RECONNECT_DELAY);
#endif
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(SSID, WIFI_PASSWORD);
#ifdef ESP8266
  //WiFi.printDiag(Serial);
#endif
}

void printWifiInfo(unsigned long wifiConnectStart) {
  unsigned long connectTime = millis() - wifiConnectStart;
  DEBUG_PRINT("Connected to wifi. Time to connect:");
  DEBUG_PRINTLN(connectTime);
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINT("Gateway address: ");
  DEBUG_PRINTLN(WiFi.gatewayIP());
  DEBUG_PRINT("DNS address: ");
  WiFi.dnsIP().printTo(Serial);
  DEBUG_PRINTLN("");
}
