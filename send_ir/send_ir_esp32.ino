// There are two usable serial ports on the ESP32. U1UXD and U2UXD. You must use these GPIO pins.
//
// U1UXD : GPIO9, GPIO10
// U2UXD : GPIO16, GPIO17
//
// Connect your board TX -> GPIO16, RX -> GPIO17

#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17

HardwareSerial mySerial(2); // use UART2

void setup() {
   // put your setup code here, to run once:
  Serial.begin(9600);
   
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2); 

  delay(1000);
  
  send_ir_code();
}
 
void send_ir_code() {
  Serial.println("Send IR code ..");
   
  // Format = {227, <your ir code here>}
  char irCode[] = "227,164,35,205,140,237,220,89,30,72,180,94,30,251,0,78,141,67,180,94,236,23,8,78,4,64,35,18,137,174,124,6,11,253,43,26,145,182,132,14,19,5,51,34,153,190,140,22,27,13,59,42,161,198,148,30,35,21,67,50,169,206,156,38,43,29,75,58,177,214,121,19,39,8,68,35,153,204,138,18,39,8,68,51,169,205,154,35,39,24,70,18,153,205,138,34,23,8,68,35,152,205,138,34,39,24,69,51,169,205,154,35,39,24,69,51,169,188,154,34,39,8,69,51,152,204,153,34,38,24,52,51,169,205,154,35,39,24,69,51,169,205,154,35,39,24,69,51,169,205,154,35,39,24,69,51,169,205,154,34,39,23,52,37,234";
  uint8_t irArray[512];
  unsigned int idx = 0;
  char* str = strtok(irCode, ",");
  
  while (str != NULL) {
      uint8_t ir = (uint8_t)atoi(str + '\0');
      irArray[idx++] = ir;
      str = strtok(NULL, ",");
  }
  
  // Write the array to Serial port.
  mySerial.write((uint8_t*)irArray, idx);
    
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

}
