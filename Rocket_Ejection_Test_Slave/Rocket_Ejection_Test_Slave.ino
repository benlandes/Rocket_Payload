#include <Wire.h>

void setup() {
   //Xbee
  Serial1.begin(9600); 
  while (!Serial1)  {
  }
  Serial.begin(9600);           // start serial for output*/
}

void loop() {
  //Serial1.println("Received");
  //Serial.println("Received");
  // put your main code here, to run repeatedly:
  if (Serial1.available() > 0) {
    
     Serial1.println("Received");
     //Serial.println("Received");
    // read the oldest byte in the serial buffer:
    char incomingByte = Serial1.read();
    // if it's a capital H (ASCII 72), turn on the LED:
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(incomingByte);        // sends five bytes
    Wire.endTransmission();
  }
  
}
