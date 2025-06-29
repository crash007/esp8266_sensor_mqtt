#ifndef SENSOR_H
#define SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "Settings.h" // For ds18b20Address
//#include <DHT.h>

extern OneWire oneWire;
extern DallasTemperature sensors;
//extern DHT dht;

void printAddress(DeviceAddress deviceAddress);
float readDS18B20();
//float readDhtHumidity();
//float readDhtTemp();

#endif
