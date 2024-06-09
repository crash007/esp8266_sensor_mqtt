/*
 * rtc.h
 *
 *  Created on: 25 mars 2018
 *      Author: pewa
 */

#ifndef RTC_H_
#define RTC_H_




struct RtcData{
	uint32_t crc32;   // 4 bytes
	uint8_t channel;  // 1 byte,   5 in total
	uint8_t ap_mac[6]; // 6 bytes, 11 in total
	uint32_t ip;
	uint32_t gateway;
	uint32_t subnet;
	uint32_t dns;
	uint8_t padding;  // 1 byte,  12 in total
};

RtcData rtcData;


uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }

      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }

  return crc;
}



bool isRtcValid() {
  bool rtcValid = false;
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = calculateCRC32(((uint8_t*) &rtcData) + 4,
        sizeof(rtcData) - 4);
    if (crc == rtcData.crc32) {
      rtcValid = true;
    }
  }
  return rtcValid;

}




#endif /* RTC_H_ */
