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



const int sleepTime = 30 * 60 * 1e6;

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
            delay(100);
        }
        retries++;
    }

    char buff[8];
    snprintf(buff, sizeof(buff), "%.3f", ds18b20Temp);
    client.publish("/v1/esp12f-2/esp12f2-ds18b20", buff);
    
    delay(5);
    
//    snprintf(buff, sizeof(buff), "%.3f", dhtTemp);
//    client.publish("/v1/esp12f-1/esp12f1-dht22-temp", buff);
//    delay(5);
//    snprintf(buff, sizeof(buff), "%.3f", dhtHum);
//    client.publish("/v1/esp12f-1/esp12f1-dht22-hum", buff);
//    delay(5);
//    
    DEBUG_PRINTLN("Publish complete");
    delay(60);
    client.disconnect();
    unsigned long uploadTime = millis() - uploadStart;
    DEBUG_PRINT("Upload took: ");
    DEBUG_PRINTLN(uploadTime);
}

void run() {
    
#ifdef DEBUG

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while (! Serial); 
    DEBUG_PRINTLN("setup");
    
    DEBUG_PRINT("Wakeup reason: ");
    DEBUG_PRINTLN(ESP.getResetReason());
    WiFi.printDiag(Serial);  
#endif

    unsigned long totalTimeStart = millis();
    unsigned long wifiConnectStart = wifiBegin();
    //WiFi.mode(WIFI_OFF);
    //WiFi.forceSleepBegin();
    //delay(1);
    // WiFi.setAutoConnect(false);

   
    pinMode(SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    pinMode(SENSOR_GND_PIN, OUTPUT);
    digitalWrite(SENSOR_GND_PIN, LOW);
    delay(100);

//    dht.begin();
//    dhtHum = readDhtHumidity();
//    dhtTemp = readDhtTemp();

    
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
 run();
}

void loop() {
   
}

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

float readDS18b20() {
    unsigned long start = millis();
   
#ifdef DEBUG   
    
    // sensors.begin();

//    DeviceAddress address;
//    while (oneWire.search(address)) {
//        DEBUG_PRINT("Found DS18B20 sensor with address: ");
//        printAddress(address);
//    //    sensors.setResolution(address, 12);
//        
//        DEBUG_PRINT("Sensor Resolution: ");
//        DEBUG_PRINTLN(sensors.getResolution(address), DEC);
//
//        sensors.requestTemperaturesByAddress(address);
//        float tempC = sensors.getTempC(address);
//        
//        if (tempC == DEVICE_DISCONNECTED_C) {
//            DEBUG_PRINTLN("Error: Could not read temperature data");
//        } else {
//            DEBUG_PRINT("Temperature for device: ");
//            printAddress(address);
//            DEBUG_PRINT(" is: ");
//            DEBUG_PRINT(tempC);
//            DEBUG_PRINTLN(" °C");
//        }
//    }
#endif
     
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
    //WiFi.disconnect(true);
    //delay(1);
    //WiFi.mode(WIFI_OFF);
    //WiFi.forceSleepBegin();
    //delay(5);
    
    #ifdef DEBUG
      Serial.flush();
      //Serial.end();
    #endif
    
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    WiFi.forceSleepBegin();
    delay(1);
   // ESP.deepSleep(sleepTime, WAKE_RF_DISABLED);
    ESP.deepSleep(sleepTime, WAKE_RF_DEFAULT);
   
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
   //WiFi.printDiag(Serial);
    //WiFi.forceSleepWake();
    //delay(100);
    //WiFi.printDiag(Serial);
    WiFi.persistent(false);
    delay(1);
    WiFi.mode(WIFI_STA);
    //delay(100);
    //WiFi.printDiag(Serial);
    
    if (isRtcValid()) {
        DEBUG_PRINTLN("RTC is valid.");
        WiFi.config(rtcData.ip, rtcData.gateway, rtcData.subnet, rtcData.dns);
        WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);
    } else {
        DEBUG_PRINTLN("RTC is not valid.");
        WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS); 
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
    WiFi.persistent(false);
    if (isRtcValid()) {
        WiFi.begin(SSID, WIFI_PASSWORD, rtcData.channel, rtcData.ap_mac, true);

    } else {
       WiFi.config(0U, 0U, 0U, 0U);
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
        delay(200);
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
  DEBUG_PRINTLN("Reconnecting......");
    //WiFi.printDiag(Serial);
    WiFi.disconnect();
    delay(100);
    WiFi.forceSleepBegin();
    delay(100);
    WiFi.forceSleepWake();
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(SSID, WIFI_PASSWORD);
    //WiFi.printDiag(Serial);
}

void wifiWaitConnected() {
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        retries++;

    switch(WiFi.status()) {
            case WL_NO_SHIELD:
               DEBUG_PRINTLN("No WiFi shield is present");
                break;
            case WL_IDLE_STATUS:
               DEBUG_PRINTLN("WiFi is in idle status");
                break;
            case WL_NO_SSID_AVAIL:
               DEBUG_PRINTLN("No SSID available - check your SSID");
                break;
            case WL_SCAN_COMPLETED:
               DEBUG_PRINTLN("WiFi scan completed");
                break;
           
            case WL_CONNECTION_LOST:
               DEBUG_PRINTLN("Connection lost");
                break;
            case WL_DISCONNECTED:
               DEBUG_PRINT("WiFi is disconnected");
                break;
            default:
               DEBUG_PRINTLN("Unknown status");
                break;
        }

        
        if (WiFi.status() == WL_CONNECT_FAILED) {
            DEBUG_PRINTLN(
                    "Failed to connect to WiFi. Please verify credentials: ");
            deepSleep();
        }
        DEBUG_PRINT(WiFi.status());
        DEBUG_PRINTLN("...");
        if(retries == 19 && WL_NO_SSID_AVAIL == WiFi.status()){
          DEBUG_PRINTLN("NO SSID available going to sleep");
          deepSleep();
        }
        
        if (isRtcValid() && retries == 20) {
            DEBUG_PRINTLN("Quick connect not working. Trying new scan.");
            wifiReconnect();
        }

        if(retries == 100) {
           wifiReconnect();
        }

//        if(retries == 200) {
//          DEBUG_PRINTLN("Disable wifi and wait 3 sek");
//          WiFi.disconnect(true);
//          delay(100);
//          WiFi.mode(WIFI_OFF);
//          delay(100);
//          WiFi.forceSleepBegin();
//          delay(3000);
//
//          
//          WiFi.forceSleepWake();
//          delay(100);
//          WiFi.persistent(false);
//          delay(100);
//          WiFi.mode(WIFI_STA);
//          delay(100);
//          WiFi.begin(SSID, WIFI_PASSWORD);
//          
//        }
        if (retries == 300) {
            DEBUG_PRINTLN("Failed to connect to WiFi. Going to sleep.");
            deepSleep();
        }
        delay(300);
    }
}
