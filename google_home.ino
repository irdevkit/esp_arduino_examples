#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>
#include <SoftwareSerial.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#define MyApiKey "" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;


// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

SoftwareSerial mySerial(D1, D2); // RX, TX

void turnOn(String deviceId) {
  Serial.println("Send IR code ..");
  char* irCode = "152,35,...."; // TODO: Paste on IR code.
  
  uint8_t irArray[512];
  unsigned int idx = 1;
  char *str;

  irArray[0] = 227; // Sending ir command
  
  // Convert string to array
  while ((str = strtok_r(irCode, ",", &irCode)) != NULL) {
    uint8_t ir = (uint8_t)atoi(str + '\0');
    irArray[idx++] = ir;
    yield();
  }  
	
  // Write the array to Serial port.
  mySerial.write((uint8_t*)irArray, idx);

}

void turnOff(String deviceId) {
  Serial.println("Send IR code ..");
  char* irCode = "152,35,...."; // TODO: Paste off IR code.
  
  uint8_t irArray[512];
  unsigned int idx = 1;
  char *str;

  irArray[0] = 227; // Sending ir command
  
  // Convert string to array
  while ((str = strtok_r(irCode, ",", &irCode)) != NULL) {
	uint8_t ir = (uint8_t)atoi(str + '\0');
	irArray[idx++] = ir;
	yield();
  }
	
  // Write the array to Serial port.
  mySerial.write((uint8_t*)irArray, idx);

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "action.devices.commands.OnOff") { // Switch 
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);  

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  mySerial.begin(9600); // Connect to ir controller


  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
}
