#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H

#include "rtc.h"

void deepSleep();
void saveApChannelBssid();
extern bool isRtcValid();

#endif
