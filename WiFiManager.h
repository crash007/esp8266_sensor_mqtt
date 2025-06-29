#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

unsigned long wifiBegin();
void wifiWaitConnected();
void wifiReconnectCachedBssid();
void wifiReconnect();
void printWifiInfo(unsigned long wifiConnectStart);

#endif
