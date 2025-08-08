#include "Sensor.h"
#include "Settings.h"
#include "Debug.h"
#include <algorithm>

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
Adafruit_BME280 bme;
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
#ifdef SENSOR_GND_PIN
  pinMode(SENSOR_GND_PIN, OUTPUT);
  digitalWrite(SENSOR_GND_PIN, LOW);
#endif  
  delay(100);

//#ifdef DEBUG   
//   sensors.begin();
//   DeviceAddress address;
//   while (oneWire.search(address)) {
//     DEBUG_PRINT("Found DS18B20 sensor with address: ");
//     printAddress(address);
//     sensors.setResolution(address, 12);
//     DEBUG_PRINT("Sensor Resolution: ");
//     DEBUG_PRINTLN(sensors.getResolution(address), DEC);
//     sensors.requestTemperaturesByAddress(address);
//     float tempC = sensors.getTempC(address);
//     if (tempC == DEVICE_DISCONNECTED_C) {
//       DEBUG_PRINTLN("Error: Could not read temperature data");
//     } else {
//       DEBUG_PRINT("Temperature for device: ");
//       printAddress(address);
//       DEBUG_PRINT(" is: ");
//       DEBUG_PRINT(tempC);
//       DEBUG_PRINTLN(" °C");
//     }
//   }
//#endif

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


//BME 280
bool initBME280() {
  pinMode( BME_280_POWER_ENABLE_PIN,OUTPUT);
  digitalWrite(BME_280_POWER_ENABLE_PIN, HIGH); 
  delay(100);
   Wire.begin(21, 22);
  bool status = bme.begin(0x76); // Eller 0x77
  if (!status) {
    DEBUG_PRINTLN("BME280 init failed!");
  } else {
    DEBUG_PRINTLN("BME280 init success.");
  }
  return status;
}

float readBmeTemperature() {
  float temp = bme.readTemperature(); // °C
  DEBUG_PRINT("BME280 Temp: "); DEBUG_PRINTLN(temp);
  return temp;
}

float readBmeHumidity() {
  float hum = bme.readHumidity(); // %
  DEBUG_PRINT("BME280 Humidity: "); DEBUG_PRINTLN(hum);
  return hum;
}

float readBmePressure() {
  float pressure = bme.readPressure() / 100.0F; // hPa
  DEBUG_PRINT("BME280 Pressure: "); DEBUG_PRINTLN(pressure);
  return pressure;
}


float readBatteryVoltage() {
  uint16_t samples[9];

  for (int i = 0; i < 9; ++i) {
    samples[i] = analogRead(BATTERY_VOLTAGE_PIN);
    delay(5); // Liten paus mellan avläsningar för stabilare värde
  }

  std::sort(samples, samples + 9);  

  uint16_t medianAdc = samples[4];  // Medianen är det mittersta värdet
  float voltage = (medianAdc / 4095.0) * 3.48 * 2.0;
 //float voltage = (medianAdc / 585.0) * 2.0;

  DEBUG_PRINT("Battery ADC median: "); DEBUG_PRINTLN(medianAdc);
  DEBUG_PRINT("Battery voltage: "); DEBUG_PRINTLN(voltage);
  return voltage;
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
