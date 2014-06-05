
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// Use software serial for GPS (Xbee using hardware serial
SoftwareSerial softSerial(11,12);
Adafruit_GPS GPS(&softSerial);

void setup() {
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  GPS.begin(9600);
}

uint32_t timer = millis();
void loop() {
  GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) return;
  }
  
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();
  
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", "); 
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
  }
}
