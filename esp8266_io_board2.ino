#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "rtc.h"
#include "Settings.h"

// Uncomment the following line to enable debugging
#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

#define SENSOR_POWER_PIN 13
#define DHTPIN 4
#define DHTTYPE DHT22
#define ONE_WIRE_BUS_PIN 5

const int sleepTime = 15 * 60 * 1e6;

//DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

DeviceAddress ds18b20Address = DS18B20_ADDRESS;
float ds18b20Temp;
float dhtHum;
float dhtTemp;

void mqttUpload(float ds18b20Temp, float dhtTemp, float dhtHum) {
    int retries = 0;

    unsigned long uploadStart = millis();
    client.setServer(MQTT_SERVER, 1883);

    while (!client.connected()) {
        DEBUG_PRINT("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
            DEBUG_PRINTLN("connected");
        } else {
            DEBUG_PRINT("failed, rc=");
            DEBUG_PRINTLN(client.state());

            if (retries == 1) {
                wifiReconnectCachedBssid(); //After 2 failures to connect do a reconnect with a dhcp request.
            }

            if (retries == 3) {
                //Unable to connect
                deepSleep();
            }
            delay(500);
        }
        retries++;
    }

    char buff[8];
    snprintf(buff, sizeof(buff), "%.3f", ds18b20Temp);
    client.publish("/v1/esp12f-1/esp12f1-ds18b20", buff);
    delay(5);
    snprintf(buff, sizeof(buff), "%.3f", dhtTemp);
    client.publish("/v1/esp12f-1/esp12f1-dht22-temp", buff);
    delay(5);
    snprintf(buff, sizeof(buff), "%.3f", dhtHum);
    client.publish("/v1/esp12f-1/esp12f1-dht22-hum", buff);
    delay(5);
    DEBUG_PRINTLN("Publish complete");
    delay(60);
    client.disconnect();
    unsigned long uploadTime = millis() - uploadStart;
    DEBUG_PRINT("Upload took: ");
    DEBUG_PRINTLN(uploadTime);
}

void run() {
    unsigned long totalTimeStart = millis();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
//    delay(1);
#ifdef DEBUG
    Serial.begin(9600);
    DEBUG_PRINTLN("setup");
#endif
    pinMode(SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    //delay(550);

//    dht.begin();
//    dhtHum = readDhtHumidity();
//    dhtTemp = readDhtTemp();

    unsigned long wifiConnectStart = wifiBegin();
    ds18b20Temp = readDS18b20();
    digitalWrite(SENSOR_POWER_PIN, LOW);
    wifiWaitConnected();
    printWifiInfo(wifiConnectStart);

    mqttUpload(ds18b20Temp, dhtTemp, dhtHum);
    //Successfull connection, saving connection info.
    saveApChannelBssid();
    DEBUG_PRINT("Total time:");
    DEBUG_PRINTLN(millis() - totalTimeStart);
    DEBUG_PRINTLN("");
    deepSleep();
}

void setup() {

}

void loop() {
    run();
}

void printAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16)
            DEBUG_PRINT("0");
        DEBUG_PRINT(deviceAddress[i], HEX);
        DEBUG_PRINT(", ");
    }
    DEBUG_PRINTLN("");
}

float readDS18b20() {
    unsigned long start = millis();
    //sensors.begin();
    /*DeviceAddress address;
     oneWire.search(address);
     printAddress(address);
     printAddress(ds18b20Address);
     sensors.setResolution(ds18b20Address, 12);

     DEBUG_PRINT("Sensor Resolution: ");
     DEBUG_PRINTLN(sensors.getResolution(ds18b20Address), DEC);
     DEBUG_PRINTLN();
     */
    sensors.setWaitForConversion(true);
    sensors.setCheckForConversion(false);
    sensors.requestTemperaturesByAddress(ds18b20Address);
    float ds18b20Temp = sensors.getTempC(ds18b20Address);
    unsigned long readTime = millis() - start;
    DEBUG_PRINT("Ds18b20 temperature is: ");
    DEBUG_PRINT(ds18b20Temp);
    DEBUG_PRINT(", time: ");
    DEBUG_PRINTLN(readTime);
    return ds18b20Temp;
}

//float readDhtHumidity() {
//    unsigned long start = millis();
//    float dhtHum = dht.readHumidity();
//    unsigned long readTime = millis() - start;
//    DEBUG_PRINT("DHT22 humidity: ");
//    DEBUG_PRINT(dhtHum);
//    DEBUG_PRINT(", time: ");
//    DEBUG_PRINTLN(readTime);
//    return dhtHum;
//}
//
//float readDhtTemp() {
//    unsigned long start = millis();
//    float dhtTemp = dht.readTemperature();
//    unsigned long readTime = millis() - start;
//    DEBUG_PRINT("DHT22 Temp: ");
//    DEBUG_PRINT(dhtTemp);
//    DEBUG_PRINT(", time: ");
//    DEBUG_PRINTLN(readTime);
//    return dhtTemp;
//}

void deepSleep() {
    DEBUG_PRINTLN("Going to sleep.");
    Serial.flush();
    Serial.end();
    WiFi.disconnect(true);
    delay(1);
    ESP.deepSleep(sleepTime, WAKE_RF_DISABLED);

    //delay(11000);
}

void saveApChannelBssid() {
    rtcData.channel = WiFi.channel();
    memcpy(rtcData.ap_mac, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)

    rtcData.ip = WiFi.localIP();
    rtcData.subnet = WiFi.subnetMask();
    rtcData.gateway = WiFi.gatewayIP();
    rtcData.dns = WiFi.dnsIP();

    rtcData.crc32 = calculateCRC32(((uint8_t*) (&rtcData)) + 4,
            sizeof(rtcData) - 4);
    ESP.rtcUserMemoryWrite(0, (uint32_t*) (&rtcData), sizeof(rtcData));
}

void printWifiInfo(unsigned long wifiConnectStart) {
    unsigned long connectTime = millis() - wifiConnectStart;
    DEBUG_PRINT("Connected to wifi. Time to connect:");
    DEBUG_PRINTLN(connectTime);
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINT("Gateway address: ");
    DEBUG_PRINTLN(WiFi.gatewayIP());
    DEBUG_PRINT("DNS address: ");
    #ifdef DEBUG
      WiFi.dnsIP().printTo(Serial);
    #endif
    DEBUG_PRINTLN("");
}

unsigned long wifiBegin() {
    unsigned long wifiConnectStart = millis();
    WiFi.forceSleepBegin();
    delay(15);
    WiFi.forceSleepWake();
    delay(15);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    delay(50);
    if (isRtcValid()) {
        DEBUG_PRINTLN("RTC is valid.");
        WiFi.config(rtcData.ip, rtcData.gateway, rtcData.subnet, rtcData.dns);
        WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
    } else {
        DEBUG_PRINTLN("RTC is not valid.");
        WiFi.begin(SSID, WIFI_PASSWORD);
    }
    return wifiConnectStart;
}

void wifiReconnectCachedBssid() {
    WiFi.disconnect();
    delay(10);
    WiFi.forceSleepBegin();
    delay(10);
    WiFi.forceSleepWake();
    delay(10);

    if (isRtcValid()) {
        WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);

    } else {
        WiFi.begin(SSID, WIFI_PASSWORD);
    }

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        retries++;
        if (WiFi.status() == WL_CONNECT_FAILED) {
            DEBUG_PRINTLN(
                    "Failed to connect to WiFi. Please verify credentials: ");
            deepSleep();
        }
        DEBUG_PRINTLN("...");
        if (retries == 100) {
            DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
            deepSleep();
        }
        delay(100);
    }

    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINT("Gateway address: ");
    DEBUG_PRINTLN(WiFi.gatewayIP());
    DEBUG_PRINT("DNS address: ");
    #ifdef DEBUG
      WiFi.dnsIP().printTo(Serial);
    #endif
    DEBUG_PRINTLN("");
}

void wifiReconnect() {
    WiFi.disconnect();
    delay(10);
    WiFi.forceSleepBegin();
    delay(10);
    WiFi.forceSleepWake();
    delay(10);
    WiFi.begin(SSID, WIFI_PASSWORD);
}

void wifiWaitConnected() {
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        retries++;
        if (WiFi.status() == WL_CONNECT_FAILED) {
            DEBUG_PRINTLN(
                    "Failed to connect to WiFi. Please verify credentials: ");
            deepSleep();
        }
        DEBUG_PRINTLN("...");
        if (isRtcValid() && retries == 100) {
            DEBUG_PRINTLN("Quick connect not working. Trying new scan.");
            wifiReconnect();
        }
        if (retries == 400) {
            DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
            deepSleep();
        }
        delay(200);
    }
}
