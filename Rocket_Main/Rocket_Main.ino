

#include <SFE_BMP180.h>
#include <Wire.h>


//Program Input
#define MAIN_TARGET_ALT 2000 //ft
#define LAUNCH_ACCEL 4 //G

//Safety Timers
#define MACH_DELAY 100 //sec
#define DROGUE_LOW_WIN 150 //Sec
#define DROGUE_HIGH_WIN 300 //Sec
#define MAIN_LOW_WIN 250 //Sec
#define MAIN_HIGH_WIN 400 //Sec

//Ports
const int accelPort = 0; //Analog

//Calibration
const double SEAP = 1013.25; //mbar

//States
#define BEFORE_ARMING 0
#define BEFORE_LAUNCH 1
#define DURING_MACH_DELAY 2
#define BEFORE_APOGEE 3
#define BEFORE_MAIN 4
#define BEFORE_LANDING 5
#define AFTER_LANDING 6

int currentStatus = BEFORE_ARMING;
SFE_BMP180 pressure;

//Sensor Offsets
double voltOffset;
double groundAlt; //ft


void setup() {
  
  //Ejection Pins
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
  
  //For communicating with slave xbee
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event

  pressure.begin();

}

//Receiving data from slave
void receiveEvent(int howMany)
{

  while(0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  Serial.println();         // Newline
}

double g, alt;
unsigned long launchTime;
float maxAlt = 0;
uint32_t timer = millis();
void loop() {
  //// Gather Data ////
  Serial.println("h1");
  //Accelerometer
  double voltageAccel = ((double) analogRead(accelPort))*5/1024 - voltOffset;
  g = (voltageAccel-2.5)/0.008;
 // double accel = g*32.2; //Acceleration (ft/s)
  Serial.println("h2");
 
  //Pressure, Temperature, and altitude
  double temp, pres, alt;
  Serial.println("h3");
  //Getting temperature increases acuracy of pressure calculation
  char status = pressure.startTemperature();
  if (status != 0)
  {
    Serial.println("h3");
    // Wait for the measurement to complete:
    delay(status);
    pressure.getTemperature(temp);
    Serial.println("h5");
    status = pressure.startPressure(3); //3 - High Resolution
    if(status != 0){
      Serial.println("h6");
      delay(status);
      status = pressure.getPressure(pres,temp);
      if(status !=0){
        alt = pressure.altitude(pres,SEAP)*3.28084,0; //ft
        pres = pres*2.09; // (lb/ft^2)
        temp = (9.0/5.0)*temp+32.0; //(deg F)
      }
      
    }
  }
  
  
  Serial.println("h7");
  //// Update Status ////
  int previousStatus = currentStatus;
  updateStatus();
  
  //Check if ejection charges need to be fired
  
  //Save data to sd card
  String dataString = String(currentStatus)+","+String(millis())+","+String(g)+","+String(temp)+","+String(pres)+","+String(alt);
  
  char dataChar[dataString.length()+1];
  dataString.toCharArray(dataChar,dataString.length()+1);
  Serial.println(dataChar);
  Serial.println("h8");
  
  Wire.beginTransmission(5); // transmit to device #4
  Wire.write(dataChar);        // sends five bytes
  Wire.endTransmission();    // stop transmitting
  
  Serial.println("h9");
  delay(250);
  //fireCharge(7);
  //fireCharge(6);
}
void setOffsets(){
  voltOffset =  ((double) analogRead(accelPort))*5/1024 - 2.5;
  groundAlt = alt;
}
void updateStatus(){
  //Check Arming Switch
  if(analogRead(1) < 512 && currentStatus != BEFORE_ARMING){
    currentStatus = BEFORE_ARMING;
  }
  
  if(currentStatus == BEFORE_ARMING){
    if(analogRead(1) > 512){
      currentStatus = BEFORE_LAUNCH;
      
      //Zero out sensors
      setOffsets();
    }
  }else if(currentStatus == BEFORE_LAUNCH){
    //If acceleration is greater than
    //expected launch trigger acceleration
    if(g > LAUNCH_ACCEL){
      launchTime = millis();
      currentStatus = DURING_MACH_DELAY;
    }
  }else if(currentStatus == DURING_MACH_DELAY){
    if(millis() - launchTime > MACH_DELAY){
       currentStatus = BEFORE_APOGEE;
    }
  }else if(currentStatus == BEFORE_APOGEE){
    if(maxAlt < alt - groundAlt){
      maxAlt = alt - groundAlt;
    }
    
    
    if(maxAlt - 30 > alt - groundAlt){
      fireCharge(6);
      currentStatus = BEFORE_MAIN;
    }
  }else if(currentStatus == BEFORE_MAIN){
    if(MAIN_TARGET_ALT -30 > alt - groundAlt){
      fireCharge(7);
    } 
  }else if(currentStatus == BEFORE_LANDING){
    
  }
}

void fireCharge(int port){
  digitalWrite(port, HIGH);   // sets the LED on
  delay(1000);                  // waits for a second
  digitalWrite(port, LOW);    // sets the LED off
}

