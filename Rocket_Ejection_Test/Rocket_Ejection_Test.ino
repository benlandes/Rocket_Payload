#include <Wire.h>

bool shouldFireForward = false;
bool shouldFireAft = false;
void setup() {
  // put your setup code here, to run once:
  //Ejection Pins
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  
  //For communicating with slave xbee
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(250);
  if(shouldFireForward){
    fireCharge(6);
    shouldFireForward = false;
  }else if(shouldFireAft){
    fireCharge(7);
    shouldFireAft = false;
  }
}
void receiveEvent(int howMany)
{

  while(0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    if(c == '0'){ //Forward
      shouldFireForward = true;
    }else if(c == '1'){ //Aft
      shouldFireAft = true;
    }
    Serial.print(c);         // print the character
  }
  Serial.println();         // Newline
}
void fireCharge(int port){
  digitalWrite(port, HIGH);   // sets the LED on
  delay(70);                  // waits for a second
  digitalWrite(port, LOW);    // sets the LED off
}
