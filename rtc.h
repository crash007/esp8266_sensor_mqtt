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
	uint8_t padding;  // 1 byte,  12 in total
};


#endif /* RTC_H_ */
