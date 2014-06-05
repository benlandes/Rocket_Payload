#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

//For Unit Testing
#define USE_TEST_INPUT false
#define TEST_FILE "TEST1.csv"

const int chipSelect = 4;
File dataFile;
File testFile;
SoftwareSerial softSerial(11, 12);
Adafruit_GPS GPS(&softSerial);

void setup() {
  delay(3000);
  // put your setup code here, to run once:
  
  pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect)) Serial.println("SD Fail");
 
  //Open up log file
  char fileName[13] = "data0001.csv"; //includes null byte
  int fileCount = 1;
  while(SD.exists(fileName)){
    fileCount++;
    sprintf(fileName, "data%04d.csv", fileCount);  
  }
  Serial.println(fileName);
  dataFile = SD.open(fileName, FILE_WRITE);
  if (! dataFile) Serial.println("SD Open Log Fail");
  
  //Print Header
  dataFile.println("Lat, Lon, GPS h, State, t(sec),a(m/s^2),T(C),P(mbar),h(m)");
  
  //Open Test File if Needed
  if(USE_TEST_INPUT){
    testFile = SD.open(TEST_FILE);
    if (!testFile) Serial.println("SD Open Test Fail");
  }
  
  
  //GPS Setup
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  GPS.begin(9600);
  
   //Xbee
  Serial1.begin(9600); 
  while (!Serial1)  {
  }
  
  //For receiving data to save to sd card from master
  Wire.begin(5);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output*/
}
void receiveEvent(int howMany)
{
  Serial1.print(GPS.latitude, 4); Serial1.print(GPS.lat); Serial1.print(",");  
  Serial1.print(GPS.longitude, 4); Serial1.print(GPS.lon); Serial1.print(","); 
  Serial1.print(GPS.altitude); 
  Serial1.print(",");
    
  dataFile.print(GPS.latitude, 4); dataFile.print(GPS.lat); dataFile.print(",");  
  dataFile.print(GPS.longitude, 4); dataFile.println(GPS.lon); dataFile.print(","); 
  dataFile.print(GPS.altitude); 
  dataFile.print(",");

  while(Wire.available()){
    char c = Wire.read();
    dataFile.print(c);
    Serial1.print(c);
  }
  /*
  int size = Wire.available();
  if(size == 1 && USE_TEST_INPUT){
    //Reset at begining of file
    if(Wire.read() == 'R') testFile = SD.open(TEST_FILE);
  }
  
  char dataChar[size+10];
  for(int i = 0; i < size; i++){
    dataChar[i] = Wire.read();
  }
  */
  /*char totalChar[size+50];
  sprintf(totalChar,"%s,%f%c,%f%c,%f",dataChar,GPS.latitude,GPS.lat,GPS.longitude,GPS.lon,GPS.altitude);
  //Serial.println(totalChar);  
  Serial1.println(totalChar);
  dataFile.println(totalChar);
  dataFile.flush();*/
  
  
  

  dataFile.println("");
  Serial1.println("");
  dataFile.flush();
  
  
  
}
uint32_t timer = millis();
void loop() {
  //Wire.beginTransmission(4); // transmit to device #4
  //Wire.write("Test");        // sends five bytes
  //Wire.endTransmission();
  
  // put your main code here, to run repeatedly:
  //GPS
   GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) return;
  }
  
  
  if (millis() - timer > 2000) { 
    
    timer = millis(); // reset the timer
    
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    //Serial1.println("Test");
    //dataFile.println("Test");
    //dataFile.flush();
    //Serial.print(GPS.hour, DEC); Serial.print(':');
    /*Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", "); 
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);*/
    //char test = GPS.lat;
    //Serial.println(test);
    //char test3 = 'A';
    //char test2[10];
    //sprintf(test2,"-%c%c-",test3,test);
    //Serial.println(test2);
    //Serial.println(GPS.lat);
  }
  
  //Will read test input file and send it to master
  if(USE_TEST_INPUT){
    if(testFile.available()){
      
      while (testFile.available()) {
        Wire.beginTransmission(4);
         // transmit to device #4
        char c = testFile.read();
        Serial.print(c);
        
        Wire.write(c);        // sends five bytes
        
        
        Wire.endTransmission();
        if(c == '\n')break;
        
      }
      
      
    }else{
      //testFile = SD.open(TEST_FILE);
    }
    delay(500);
    
  }
  
  
  

}
