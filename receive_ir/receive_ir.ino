#include <Arduino.h>
#include <SoftwareSerial.h>

// IR Controller is connected to WeMos D1, D2
//SoftwareSerial mySerial(D1, D2); // RX, TX

// Comment above and uncomment this line if you are using Maker Model
//SoftwareSerial mySerial(D5, D7); // RX, TX

void start_learning_mode();
void read_ir_signal();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
   
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  mySerial.begin(9600); // Start communicating with IR controller

  start_learning_mode();
}
 
void loop() {
  if (mySerial.available()) {
      // Read the data from buffer
      read_ir_signal();
  }
}

void start_learning_mode() {
  Serial.println("Turnning on learning mode ..");

  // Start learning mode by sending 224 (0xe0 in hex) to the ir controller
  uint8_t data[] = {0xe0};
  mySerial.write((uint8_t*)data, sizeof(data));

  // Read the device response
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
    Serial.println("Ready to record the remote. Press any button now..");
  }
}

void read_ir_signal() {
  int len = 0;
  int c;  
  unsigned long timeout = 700;
  unsigned long start = millis();
  
  int buffer[512];
  memset(buffer, 0, sizeof(buffer));
      
  while ((millis() - start < timeout)) {
    if (mySerial.available()) {
      c = mySerial.read();
      buffer[len++] = c;
      //Serial.print(c);
      //Serial.println(",");
    }
    yield();
  }

 String ir_signal = "";
 unsigned int num = 0;

 for (int idx = 0; idx < len; idx++) {
     ir_signal += buffer[idx];
     
     // If not the last index, append "," to string
     if(idx+1 != len ) {
       ir_signal += ",";
     }

     // Ignore the last digit in the array. It is the checksum
     if(idx != len -1) {
         num += buffer[idx];
     }
 }

 byte received_checksum = (byte)num;
 int ir_signal_checksum = buffer[len -1];

 if(received_checksum == ir_signal_checksum) {
    Serial.println("Your ir signal:");
    Serial.println(ir_signal);
 } else {
    Serial.println("Invalid checksum:");
 }
}
