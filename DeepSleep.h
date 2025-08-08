#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H

#include "rtc.h"

void deepSleep();
void saveApChannelBssid();
void logWakeUpReason();
extern bool isRtcValid();

#endif
