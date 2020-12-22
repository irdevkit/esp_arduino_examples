

// Sinric Pro + IRDEVKIT example for IRDEVKIT's Maker model
// v1.0

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "SinricProTemperaturesensor.h"


#define SEALEVELPRESSURE_HPA (1013.25)
#define HOSTNAME    		"hostname"
#define EVENT_WAIT_TIME   	60000               // send event every 60 seconds
#define BAUD_RATE   		9600

#define APP_KEY     "" // TODO Get it from https://portal.sinroc.pro
#define APP_SECRET  "" // TODO Get it from https://portal.sinroc.pro

#define TEMPERATURE_SENSOR_ID   "" // TODO: Get it from https://portal.sinroc.pro
#define SWITCH_ID               "" // TODO: Get it from https://portal.sinroc.pro

#define WIFI_SSID   "" // TODO: Update WiFI SSID 
#define WIFI_PASS   "" // TODO: Update WiFI SSID 

Adafruit_BME280 bme; // I2C
SoftwareSerial mySerial(D5, D7); // RX, TX

bool myPowerState = false;
unsigned long lastBtnPress = 0;
unsigned long startTime = millis();

bool deviceIsOn;                              // Temeprature sensor on/off state
float temperature;                            // actual temperature
float humidity;                               // actual humidity
float lastTemperature;                        // last known temperature (for compare)
float lastHumidity;                           // last known humidity (for compare)
unsigned long lastEvent = (-EVENT_WAIT_TIME); // last time event has been sent

void setupOTA() {
  // onStart contains SinricPro related stuff to disconnect automaticly from SinricPro when update starts!
  ArduinoOTA.onStart([]() { SinricPro.stop(); Serial.printf("Start updating %s\r\n", ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "spiffs"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.setHostname(HOSTNAME);  
  ArduinoOTA.begin();
}

void setupWiFi() {
  WiFi.hostname(HOSTNAME);
  
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
  Serial.printf("[WiFi]: Hostname is \"%s\"\r\n", HOSTNAME);
}

bool onTemperatureSenosrPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  return true; // request handled properly
}

bool onSwitchPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");

  if(state) {
    uint8_t irArray[] = {227, <your ircode> }; // TODO: Must be followed by 227,
    mySerial.write((uint8_t*)irArray, sizeof(irArray));
  } else {
    uint8_t irArray[] = {227, <your ircode> }; // TODO: Must be followed by 227,
    mySerial.write((uint8_t*)irArray, sizeof(irArray));
  }

  Serial.println("Response: ");
  
  int len = 0;
  int c;  
  unsigned long timeout = 1000;
  unsigned long start = millis();
        
  while ((millis() - start < timeout)) {
    if (mySerial.available()) {
      c = mySerial.read();

      Serial.print(c, HEX); 
      Serial.print(",");
    }
    yield();
  }  
  
  return true; // request handled properly
}
 
// setup function for SinricPro
void setupSinricPro() {
  // add device to SinricPro
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onSwitchPowerState); 

  SinricProTemperaturesensor &mySensor = SinricPro[TEMPERATURE_SENSOR_ID];
  mySensor.onPowerState(onTemperatureSenosrPowerState);
  
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  
  // setup SinricPro
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setupTemperatureSensor() {
  if (!bme.begin(BME280_ADDRESS_ALTERNATE)) { 
    Serial.println("Could not find a valid BME280 sensor!");
    while (1)
      ;
  }
} 

void handleTemperaturesensor() {
  unsigned long actualMillis = millis();
  if (actualMillis - lastEvent < EVENT_WAIT_TIME) return; //only check every EVENT_WAIT_TIME milliseconds

  Serial.println("Temperature: " + String(bme.readTemperature(), 2) + " *C");
  Serial.println("Pressure: " + String(bme.readPressure() / 100.0F, 2) + " hPa");
  Serial.println("Altitude: " + String(bme.readAltitude(SEALEVELPRESSURE_HPA), 2) + " m");
  Serial.println("Humidity: " + String(bme.readHumidity(), 2) + " %");
  Serial.println("");
  
  temperature = bme.readTemperature();          // get actual temperature
  humidity = bme.readHumidity();                // get actual humidity

  if (isnan(temperature) || isnan(humidity)) { // reading failed... 
    Serial.printf("BME reading failed!\r\n");  // print error message
    return;                                    // try again next time
  } 

  if (temperature == lastTemperature || humidity == lastHumidity) return; // if no values changed do nothing...

  SinricProTemperaturesensor &mySensor = SinricPro[TEMPERATURE_SENSOR_ID];  // get temperaturesensor device
  bool success = mySensor.sendTemperatureEvent(temperature, humidity); // send event

  if (success) {  // if event was sent successfuly, print temperature and humidity to serial
    Serial.printf("Temperature: %2.1f Celsius\tHumidity: %2.1f%%\r\n", temperature, humidity);
  } else {  // if sending event failed, print error message
    Serial.printf("Something went wrong...could not send Event to server!\r\n");
  }

  lastTemperature = temperature;  // save actual temperature for next compare
  lastHumidity = humidity;        // save actual humidity for next compare
  lastEvent = actualMillis;       // save actual time for next compare
}
 
void setup() {
  Wire.begin();   
  Serial.begin(BAUD_RATE);
  mySerial.begin(9600); // Start communicating with IR controller

  setupTemperatureSensor();
  setupWiFi();
  setupSinricPro();
  setupOTA();
}
 
void loop() {
  // put your main code here, to run repeatedly:
  handleTemperaturesensor();
  SinricPro.handle();
  ArduinoOTA.handle();
}
