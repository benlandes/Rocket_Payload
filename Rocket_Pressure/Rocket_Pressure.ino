#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 pressure;
#define SEAP 1013.25 //mbar

void setup() {
  Serial.begin(9600);
  pressure.begin();
}

void loop() {
  double T,P;
  
  //Getting temperature increases acuracy of pressure calculation
  char status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    pressure.getTemperature(T);
    Serial.print("temperature: ");
    Serial.print(T,2);
    Serial.print(" deg C, ");
    Serial.print((9.0/5.0)*T+32.0,2);
    Serial.println(" deg F");
    
    status = pressure.startPressure(3);
    if(status != 0){
      delay(status);
      status = pressure.getPressure(P,T);
      if(status !=0){
        double alt = pressure.altitude(P,SEAP);
        Serial.print("Altitude: ");
        Serial.print(alt,0);
        Serial.print(" meters, ");
        Serial.print(alt*3.28084,0);
        Serial.println(" feet");
      }
    }
  }
  
  delay(1000);
}
