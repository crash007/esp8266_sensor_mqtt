#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "rtc.h"
#include "Settings.h"

#define SENSOR_POWER_PIN 13
#define DHTPIN 4
#define DHTTYPE DHT22
#define ONE_WIRE_BUS_PIN 5


const int sleepTime = 1 * 15 * 1e6;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

IPAddress ip(192, 168, 0, 155);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient espClient;
PubSubClient client(espClient);

DeviceAddress ds18b20Address = DS18B20_ADDRESS;
float ds18b20Temp;
float dhtHum;
float dhtTemp;

RtcData rtcData;


void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum) {
	int retries = 2;

	unsigned long uploadStart = millis();
	client.setServer(MQTT_SERVER, 1883);

	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
			Serial.println("connected");
		} else {

			Serial.print("failed, rc=");
			Serial.println(client.state());

			if (retries == 0) {
				deepSleep();
			}
			delay(1000);
		}
		retries--;
	}

  char buff[8];
  snprintf (buff, sizeof(buff), "%.3f", ds18b20Temp);
	client.publish("/v1/esp12f-1/esp12f1-ds18b20", buff);
	delay(5);
  snprintf (buff, sizeof(buff), "%.3f", dhtTemp);
	client.publish("/v1/esp12f-1/esp12f1-dht22-temp", buff);
	delay(5);
  snprintf (buff, sizeof(buff), "%.3f", dhtHum);
	client.publish("/v1/esp12f-1/esp12f1-dht22-hum", buff);
	delay(5);
	Serial.println("Publish complete");
	delay(60);
	client.disconnect();
	unsigned long uploadTime = millis() - uploadStart;
	Serial.print("Upload took: ");
	Serial.println(uploadTime);
}


void run() {
	unsigned long totalTimeStart = millis();
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1);
	Serial.begin(9600);
	Serial.println("setup");

	pinMode(SENSOR_POWER_PIN, OUTPUT);
	digitalWrite(SENSOR_POWER_PIN, HIGH);
	delay(500);
	dht.begin();
	dhtHum = readDhtHumidity();
	dhtTemp = readDhtTemp();
	unsigned long wifiConnectStart = wifiBegin();
	ds18b20Temp = readDS18b20();
	digitalWrite(SENSOR_POWER_PIN, LOW);
	wifiWaitConnected();
	saveApChannelBssid();
	printWifiInfo(wifiConnectStart);
	
	mqttUpload(ds18b20Temp, dhtTemp, dhtHum);
	Serial.print("Total time:");
	Serial.println(millis() - totalTimeStart);
	Serial.println("");
	deepSleep();
}

void setup() {
  
}

void loop() {
	run();
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    Serial.print(", ");
    
  }
  Serial.println("");
}

float readDS18b20() {
	unsigned long start = millis();
	//sensors.begin();
  //DeviceAddress address;
	//oneWire.search(address);
  //printAddress(address);
  //printAddress(ds18b20Address);
  //sensors.setResolution(ds18b20Address, 12);
  sensors.setWaitForConversion(true);
  sensors.setCheckForConversion(false);
  //Serial.print("Sensor Resolution: ");
  //Serial.println(sensors.getResolution(ds18b20Address), DEC); 
  //Serial.println();
	sensors.requestTemperaturesByAddress(ds18b20Address); 
	float ds18b20Temp = sensors.getTempC(ds18b20Address);
	unsigned long readTime = millis()-start;
	Serial.print("Ds18b20 temperature is: ");
	Serial.print(ds18b20Temp);
	Serial.print(", time: ");
	Serial.println(readTime);
	return ds18b20Temp;
}

float readDhtHumidity() {
	unsigned long start = millis();
	float dhtHum = dht.readHumidity();
	unsigned long readTime = millis()-start;
	Serial.print("DHT22 humidity: ");
	Serial.print(dhtHum);
	Serial.print(", time: ");
	Serial.println(readTime);
	return dhtHum;
}

float readDhtTemp() {
	unsigned long start = millis();
	float dhtTemp = dht.readTemperature();
	unsigned long readTime = millis()-start;
	Serial.print("DHT22 Temp: ");
	Serial.print(dhtTemp);
	Serial.print(", time: ");
	Serial.println(readTime);
	return dhtTemp;
}

void deepSleep() {

	Serial.println("Going to sleep.");
	WiFi.disconnect(true);
	delay(1);
	ESP.deepSleep(sleepTime, WAKE_RF_DISABLED);
	//delay(15000);
}

void saveApChannelBssid() {
	rtcData.channel = WiFi.channel();
	memcpy(rtcData.ap_mac, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
	rtcData.crc32 = calculateCRC32(((uint8_t*) (&rtcData)) + 4,
			sizeof(rtcData) - 4);
	ESP.rtcUserMemoryWrite(0, (uint32_t*) (&rtcData), sizeof(rtcData));
}

void printWifiInfo(unsigned long wifiConnectStart) {
	unsigned long connectTime = millis() - wifiConnectStart;
	Serial.print("Connected to wifi. Time to connect:");
	Serial.println(connectTime);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	Serial.print("Gateway address: ");
	Serial.println(WiFi.gatewayIP());
	Serial.print("DNS address: ");
	WiFi.dnsIP().printTo(Serial);
	Serial.println("");
}

unsigned long wifiBegin() {
	unsigned long wifiConnectStart = millis();
	WiFi.forceSleepWake();
	delay(1);
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gateway, subnet, gateway);
	if (isRtcValid()) {
		Serial.println("RTC is valid.");
		WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
	} else {
		Serial.println("RTC is not valid.");
		WiFi.begin(SSID, WIFI_PASSWORD);
	}
	return wifiConnectStart;
}

void wifiWaitConnected() {
	int retries = 0;
	while (WiFi.status() != WL_CONNECTED) {
		retries++;
		if (WiFi.status() == WL_CONNECT_FAILED) {
			Serial.println(
					"Failed to connect to WiFi. Please verify credentials: ");
			deepSleep();
		}
		Serial.println("...");
		if (retries == 100) {
			Serial.println("Quick connect not working. Trying new scan.");
			WiFi.disconnect();
			delay(10);
			WiFi.forceSleepBegin();
			delay(10);
			WiFi.forceSleepWake();
			delay(10);
			WiFi.begin(SSID, WIFI_PASSWORD);
		}
		if (retries == 200) {
			Serial.println("Failed to connect to WiFi. Going to sleep.");
			deepSleep();
		}
		delay(50);
	}
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


