/*
 * rtc.h
 *
 *  Created on: 25 mars 2018
 *      Author: pewa
 */

#ifndef RTC_H_
#define RTC_H_

#include <Arduino.h> // For uint32_t, uint8_t

struct RtcData {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t ap_mac[6]; // 6 bytes, 11 in total
  uint32_t ip;
  uint32_t gateway;
  uint32_t subnet;
  uint32_t dns;
  uint8_t padding;  // 1 byte,  12 in total
};

#ifdef ESP8266
  extern RtcData rtcData; // Declared extern, defined in rtc.cpp
#else
  RTC_DATA_ATTR RtcData rtcData; // Persist in RTC memory for ESP32
#endif

uint32_t calculateCRC32(const uint8_t *data, size_t length);
bool isRtcValid();

#endif /* RTC_H_ */
