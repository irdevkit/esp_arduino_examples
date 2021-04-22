#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(D1, D2); // RX, TX

// Comment above and uncomment this line if you are using Maker Model
//SoftwareSerial mySerial(D5, D7); // RX, TX
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  mySerial.begin(9600); // Connect to ir controller

  Serial.println("Send IR code ..");
  
  // Format = {227, <your ir code here>}
  uint8_t irArray[] = {227, 88,35,53,141,237,204,109,41,82,42,95,142,249,128,79,5,72,44,95,113,133,207,109,26,74,44,95,134,245,120,2,7,249,39,22,141,178,128,10,15,1,47,30,149,186,136,18,23,9,55,38,157,194,144,26,31,17,63,46,165,202,152,34,39,25,71,54,173,210,121,18,22,7,53,51,169,205,154,35,22,7,52,34,169,205,155,70,140};
  mySerial.write((uint8_t*)irArray, sizeof(irArray));
    

  // Read the response.
  int len = 0;
  int r;
  unsigned long timeout = 700;
  unsigned long start = millis();
  int buffer[1];
  memset(buffer, 0, sizeof(buffer));
       
  while (millis() - start < timeout) {
    if (mySerial.available()) {
      buffer[0] = mySerial.read();
    }

    yield();
  }

  if(buffer[0] == 255) { // ff
    Serial.println("Error starting..");
  }
  else {
    Serial.println("Success!..");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
