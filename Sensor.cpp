#include "Sensor.h"
#include "Settings.h"
#include "Debug.h"

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
//DHT dht(DHTPIN, DHTTYPE);

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    DEBUG_PRINT("0x");
    if (deviceAddress[i] < 16)
      DEBUG_PRINT("0");
    DEBUG_PRINT(deviceAddress[i], HEX);
    if (i < 7) {
      DEBUG_PRINT(", ");
    }
  }
  DEBUG_PRINTLN("");
}

float readDS18B20() {
  unsigned long start = millis();

  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  pinMode(SENSOR_GND_PIN, OUTPUT);
  digitalWrite(SENSOR_GND_PIN, LOW);
  delay(100);

#ifdef DEBUG   
  // sensors.begin();
  // DeviceAddress address;
  // while (oneWire.search(address)) {
  //   DEBUG_PRINT("Found DS18B20 sensor with address: ");
  //   printAddress(address);
  //   sensors.setResolution(address, 12);
  //   DEBUG_PRINT("Sensor Resolution: ");
  //   DEBUG_PRINTLN(sensors.getResolution(address), DEC);
  //   sensors.requestTemperaturesByAddress(address);
  //   float tempC = sensors.getTempC(address);
  //   if (tempC == DEVICE_DISCONNECTED_C) {
  //     DEBUG_PRINTLN("Error: Could not read temperature data");
  //   } else {
  //     DEBUG_PRINT("Temperature for device: ");
  //     printAddress(address);
  //     DEBUG_PRINT(" is: ");
  //     DEBUG_PRINT(tempC);
  //     DEBUG_PRINTLN(" °C");
  //   }
  // }
#endif

  sensors.setWaitForConversion(true);
  sensors.setCheckForConversion(false);
  sensors.requestTemperaturesByAddress(ds18b20Address);
  float ds18b20Temp = sensors.getTempC(ds18b20Address);
  digitalWrite(SENSOR_POWER_PIN, LOW);

  unsigned long readTime = millis() - start;
  DEBUG_PRINT("Ds18b20 temperature is: ");
  DEBUG_PRINT(ds18b20Temp);
  DEBUG_PRINT(", time: ");
  DEBUG_PRINTLN(readTime);
  return ds18b20Temp;
}

//float readDhtHumidity() {
//  unsigned long start = millis();
//  float dhtHum = dht.readHumidity();
//  unsigned long readTime = millis() - start;
//  DEBUG_PRINT("DHT22 humidity: ");
//  DEBUG_PRINT(dhtHum);
//  DEBUG_PRINT(", time: ");
//  DEBUG_PRINTLN(readTime);
//  return dhtHum;
//}

//float readDhtTemp() {
//  unsigned long start = millis();
//  float dhtTemp = dht.readTemperature();
//  unsigned long readTime = millis() - start;
//  DEBUG_PRINT("DHT22 Temp: ");
//  DEBUG_PRINT(dhtTemp);
//  DEBUG_PRINT(", time: ");
//  DEBUG_PRINTLN(readTime);
//  return dhtTemp;
//}
