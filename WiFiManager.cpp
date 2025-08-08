#include "WiFiManager.h"
#include <DallasTemperature.h> // For DeviceAddress (via Settings.h)
#include "Settings.h"
#include "DeepSleep.h"
#include "Debug.h"
#include "rtc.h"

unsigned long wifiBegin() {
  unsigned long wifiConnectStart = millis();
  DEBUG_PRINTLN("Starting wifiBegin");
#ifdef ESP8266
  WiFi.forceSleepWake();
  delay(WIFI_RECONNECT_DELAY);
#endif
  WiFi.persistent(false);
  delay(DEEP_SLEEP_DELAY);
  WiFi.mode(WIFI_OFF); // Ensure clean WiFi state
  delay(100); // Increased delay for router stability
  WiFi.mode(WIFI_STA);
  delay(100); // Increased delay for WiFi hardware

  if (isRtcValid()) {
    DEBUG_PRINTLN("RTC is valid.");
    DEBUG_PRINT("IP: "); DEBUG_PRINTLN(IPAddress(rtcData.ip));
    DEBUG_PRINT("Gateway: "); DEBUG_PRINTLN(IPAddress(rtcData.gateway));
    DEBUG_PRINT("Subnet: "); DEBUG_PRINTLN(IPAddress(rtcData.subnet));
    DEBUG_PRINT("DNS: "); DEBUG_PRINTLN(IPAddress(rtcData.dns));
    DEBUG_PRINT("Channel: "); DEBUG_PRINTLN(rtcData.channel);
    DEBUG_PRINT("BSSID: ");
    for (int i = 0; i < 6; i++) {
      DEBUG_PRINT(rtcData.ap_mac[i], HEX); DEBUG_PRINT(":");
    }
    DEBUG_PRINTLN("");
    //WiFi.config(rtcData.ip, rtcData.gateway, rtcData.subnet, rtcData.dns);
  
    WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
  } else {
    DEBUG_PRINTLN("RTC is not valid.");
    WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0)); // Use DHCP
    delay(500); // Increased delay for DHCP stability
    WiFi.begin(SSID, WIFI_PASSWORD);
  }
  DEBUG_PRINTLN("wifiBegin completed");
  return wifiConnectStart;
}

void wifiWaitConnected() {
  int retries = 0;
  DEBUG_PRINTLN("Starting wifiWaitConnected");

  while (WiFi.status() != WL_CONNECTED) {
    retries++;
    DEBUG_PRINT("WiFi status: ");
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
      DEBUG_PRINTLN("Failed to connect to WiFi. Please verify credentials.");
      deepSleep();
    }

    if (retries == WIFI_SSID_FAIL_RETRIES && WiFi.status() == WL_NO_SSID_AVAIL) {
      DEBUG_PRINTLN("No SSID available going to sleep");
      deepSleep();
    }

    if (isRtcValid() && retries == WIFI_QUICK_CONNECT_RETRIES) {
      DEBUG_PRINTLN("Quick connect not working. Trying new scan.");
      wifiReconnect();
    }

    if (retries == WIFI_RECONNECT_RETRIES) {
      DEBUG_PRINTLN("Retrying with full reinitialization.");
      wifiReconnect();
    }

    if (retries == WIFI_MAX_RETRIES) {
      DEBUG_PRINTLN("Failed to connect to WiFi after max retries. Going to sleep.");
      deepSleep();
    }
    delay(WIFI_CONNECT_DELAY);
  }

  // Save DHCP settings to RTC if not using valid data
  if (!isRtcValid()) {
    DEBUG_PRINTLN("Saving DHCP network settings to RTC.");
    saveApChannelBssid();
  }
  DEBUG_PRINTLN("wifiWaitConnected completed");
}

void wifiReconnectCachedBssid() {
  DEBUG_PRINTLN("Starting wifiReconnectCachedBssid");
  WiFi.disconnect();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
#ifdef ESP8266
  WiFi.forceSleepBegin();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
  WiFi.forceSleepWake();
  delay(WIFI_RECONNECT_BSSID_SHORT_DELAY);
#endif
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  delay(500); // Increased delay for router stability
  WiFi.mode(WIFI_STA);
  delay(500); // Increased delay for WiFi hardware

  if (isRtcValid()) {
    DEBUG_PRINTLN("RTC is valid for reconnect.");
    WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
  } else {
    DEBUG_PRINTLN("RTC is not valid for reconnect.");
    WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0)); // Use DHCP
    delay(500); // Increased delay for DHCP stability
    WiFi.begin(SSID, WIFI_PASSWORD);
  }

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    retries++;
    if (WiFi.status() == WL_CONNECT_FAILED) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Please verify credentials.");
      deepSleep();
    }
    DEBUG_PRINTLN("...");
    if (retries == WIFI_RECONNECT_RETRIES) {
      DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
      deepSleep();
    }
    delay(WIFI_RECONNECT_BSSID_DELAY);
  }

  // Save DHCP settings to RTC if not using valid data
  if (!isRtcValid()) {
    DEBUG_PRINTLN("Saving DHCP network settings to RTC.");
    saveApChannelBssid();
  }

  DEBUG_PRINT("IP address: "); DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINT("Gateway address: "); DEBUG_PRINTLN(WiFi.gatewayIP());
  DEBUG_PRINT("DNS address: "); WiFi.dnsIP().printTo(Serial);
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("wifiReconnectCachedBssid completed");
}

void wifiReconnect() {
  DEBUG_PRINTLN("Reconnecting......");
#ifdef ESP8266
  //WiFi.printDiag(Serial);
#endif
  WiFi.disconnect();
  delay(500); // Increased delay
#ifdef ESP8266
  WiFi.forceSleepBegin();
  delay(500);
  WiFi.forceSleepWake();
  delay(500);
#endif
  WiFi.mode(WIFI_OFF);
  delay(500); // Increased delay for router stability
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  delay(500); // Increased delay for WiFi hardware
  WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0)); // Use DHCP
  WiFi.begin(SSID, WIFI_PASSWORD);
#ifdef ESP8266
  //WiFi.printDiag(Serial);
#endif
  DEBUG_PRINTLN("wifiReconnect completed");
}

void printWifiInfo(unsigned long wifiConnectStart) {
  unsigned long connectTime = millis() - wifiConnectStart;
  DEBUG_PRINT("Connected to wifi. Time to connect:"); DEBUG_PRINTLN(connectTime);
  DEBUG_PRINT("IP address: "); DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINT("Gateway address: "); DEBUG_PRINTLN(WiFi.gatewayIP());
  DEBUG_PRINT("DNS address: "); WiFi.dnsIP().printTo(Serial);
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("printWifiInfo completed");
}
