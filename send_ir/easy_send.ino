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
  uint8_t[] irCode = {227, 152,35,181,140,245,128,95,185,73,93,94,239,253,179,78,139,68,94,94,101,96,179,78,4,64,35,18,137,174,124,6,11,253,43,26,145,182,132,14,19,5,51,34,153,190,140,22,27,13,59,42,161,198,148,30,35,21,67,50,169,206,156,38,43,29,75,58,177,214,121,35,38,24,69,35,152,204,137,18,39,8,69,51,168,188,137,34,23,24,69,51,169,205,153,18,23,24,70,18,169,204,154,35,23,7,68,34,152,205,138,35,39,24,69,50,153,204,153,18,39,23,53,50,169,204,137,19,39,24,69,51,169,205,154,35,22,7,69,51,152,205,154,35,39,8,69,50,153,204,138,37,78};
  mySerial.write((uint8_t*)irCode,  sizeof(irCode));
    

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
