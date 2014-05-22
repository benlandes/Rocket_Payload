
#include <SPI.h>
#include <SD.h>


const int chipSelect = 10;

File dataFile;

void setup() {
  // Initialize SD Card
  delay(5000);
  pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect)) Serial.println("Card failed, or not present");
 
  //Open up log file
  char fileName[13] = "data0001.csv"; //includes null byte
  int fileCount = 1;
  while(SD.exists(fileName)){
    fileCount++;
    sprintf(fileName, "data%04d.csv", fileCount);  
  }
  Serial.println(fileName);
  dataFile = SD.open(fileName, FILE_WRITE);
  if (! dataFile) Serial.println("error opening file for writing");
  
  //Print Header
  dataFile.println("Voltage,g,a");
  dataFile.flush();
}

int accelOffset = -4; //Zero out acceleration reading
void loop() {


  double voltageAccel = ((double) analogRead(0)+accelOffset)*5/1024;
  double g = (voltageAccel-2.5)/0.008;
  double a = g*32.2;
  
  String dataString = String(voltageAccel)+","+String(g)+","+String(a);

  dataFile.println(dataString);
  Serial.println(dataString);
  dataFile.flush();
  
  // Take 1 measurement every 500 milliseconds
  delay(500);
}
