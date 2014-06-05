

#include <SFE_BMP180.h>
#include <Wire.h>

//For Unit Testing
#define USE_TEST_INPUT false

//Program Input
//L2

#define USE_DUAL_DEPLOYMENT true
#define SECOND_TARGET_ALT 750 //ft
#define LAUNCH_ACCEL 30 //m/s^2


//L1
/*
#define USE_DUAL_DEPLOYMENT false
#define SECOND_TARGET_ALT 0 //ft
#define LAUNCH_ACCEL 30.0 //m/s^2
*/
//Safety Timers
//L2

#define MACH_DELAY 9 //sec
#define APOGEE_LOW_WIN 15 //Sec
#define APOGEE_HIGH_WIN 60 //Sec
#define SECOND_LOW_WIN 100 //Sec
#define SECOND_HIGH_WIN 300 //Sec


//L1
/*#define MACH_DELAY 0 //sec
#define APOGEE_LOW_WIN 5.0 //Sec
#define APOGEE_HIGH_WIN 18 //Sec
#define SECOND_LOW_WIN 0 //Sec
#define SECOND_HIGH_WIN 0 //Sec
*/

//Ports
const int accelPort = 0; //Analog

//Calibration
#define SEAP 1019.0 //mbar

//Data Smoothing
#define NUM_ALT_READINGS 10
double altReadings[NUM_ALT_READINGS];
int altReadsIndex = 0;
int altReadsTotal = 0;
int altAvg = 0;

#define NUM_ACCEL_READINGS 10
double accelReadings[NUM_ACCEL_READINGS];
int accelReadsIndex = 0;
int accelReadsTotal = 0;
int accelAvg = 0;


//States
#define BEFORE_ARMING 0
#define BEFORE_LAUNCH 1
#define DURING_MACH_DELAY 2
#define BEFORE_APOGEE 3
#define BEFORE_SECOND 4 //2nd parachute
#define BEFORE_LANDING 5
#define AFTER_LANDING 6

int currentStatus = BEFORE_LAUNCH;
SFE_BMP180 pressure;

//Sensor Offsets
float voltOffset, groundAlt; //V, ft

void setup() {
  
  //Ejection Pins
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
  
  //For communicating with slave xbee
  Wire.begin(4);                // join i2c bus with address #4
  //Send Restart Test Signal to Slave if in Test Mode
  if(USE_TEST_INPUT){
    Wire.beginTransmission(5);
    Wire.write('R');        
    Wire.endTransmission();
  }
  Wire.onReceive(receiveEvent); // register event

  pressure.begin();
  
  //Will set offsets again once armed
  setOffsets();
}



double accel, alt, temp, pres;
bool armingSwitchEngaged = false;
double launchTime, time;
float maxAlt = 0;
char receiveLine[50];
int receiveI;
uint32_t timer = millis();

//Receiving data from slave
void receiveEvent(int howMany)
{
  if(USE_TEST_INPUT){
    while(Wire.available()){
       char c = Wire.read();
      
       //Serial.print(c);
       receiveLine[receiveI] = c;
       receiveI++;
       if(c == '\n'){
         //Serial.print(receiveLine);
         time = atof(strtok(receiveLine,","));
         alt = atof(strtok(NULL,","));
         accel = atof(strtok(NULL,","));
         temp = atof(strtok(NULL,","));
         pres = atof(strtok(NULL,","));
         armingSwitchEngaged = atoi(strtok(NULL,","));
         
         
         /*Serial.print("Time: "); Serial.print(time); Serial.print("(s)");
         Serial.print(" h: "); Serial.print(alt); Serial.print("(m)");
         Serial.print(" a: "); Serial.print(accel); Serial.print("(m/s^2)");
         Serial.print(" T: "); Serial.print(temp); Serial.print("(C)");
         Serial.print(" P: "); Serial.print(pres); Serial.print("(mbar)");
         if(armingSwitchEngaged) Serial.println(" Switch Engaged");
         else Serial.println(" Switch Disengaged");*/
         
         process();
         
         for(int i = 0; i < 50; i++){
           receiveLine[i] = 0;
         }
         receiveI = 0;
         
       }
       
    }  
  }else{
    
    while(0 < Wire.available()) // loop through all but the last
    {
      char c = Wire.read(); // receive byte as a character
      Serial.print(c);         // print the character
    }
    //Serial.println();         // Newline
  }
  
}
void loop() {
  
  //When unit testing data will be read from sd card instead
  //of from sensors
  if(USE_TEST_INPUT == false){
    //// Gather Data ////
    
    //Accelerometer
    double voltageAccel = ((double) analogRead(accelPort))*5/1024 - voltOffset;
    accel = -(voltageAccel-2.5)/0.008*9.81; //m/s^2
       
    //Getting temperature increases acuracy of pressure calculation
    char status = pressure.startTemperature();
    if (status != 0)
    {

      // Wait for the measurement to complete:
      delay(status);
      pressure.getTemperature(temp);
      status = pressure.startPressure(3); //3 - High Resolution
      if(status != 0){
        delay(status);
        status = pressure.getPressure(pres,temp); //mbar, celsius
        if(status != 0){
          //Uses equation for first gradiant layer (Valid for max altitude rocket will reach)
          alt = pressure.altitude(pres,SEAP); //m
        }
      }
    }
    //Safety Switch (1024 - engaged, 0 - disengaged)
    if(analogRead(1) < 512){
      armingSwitchEngaged = false;
    }else{
      armingSwitchEngaged = true;
    }
    
    time = double(millis())/1000; //Sec
    process();
    delay(500);
  }
  
}
void process(){
  //// Update Status ////
  int previousStatus = currentStatus;
  
  //Smooth out altitude data
  altReadsTotal -= altReadings[altReadsIndex];
  altReadings[altReadsIndex] = alt;
  altReadsTotal += altReadings[altReadsIndex];
  altReadsIndex++;
  
  if(altReadsIndex >= NUM_ALT_READINGS){
    altReadsIndex = 0;
  }
  
  altAvg = altReadsTotal/NUM_ALT_READINGS;
  
  //Smooth out accelerometer data
  accelReadsTotal -= accelReadings[accelReadsIndex];
  accelReadings[accelReadsIndex] = accel;
  accelReadsTotal += accelReadings[accelReadsIndex];
  accelReadsIndex++;
  
  if(accelReadsIndex >= NUM_ACCEL_READINGS){
    accelReadsIndex = 0;
  }
  
  accelAvg = accelReadsTotal/NUM_ACCEL_READINGS;
  
  updateStatus();
  
  //Check if ejection charges need to be fired
  if(USE_DUAL_DEPLOYMENT){
    if(previousStatus == BEFORE_APOGEE && currentStatus == BEFORE_SECOND){
      fireCharge(7); //Aft Charge - Drouge 
    }else if(previousStatus == BEFORE_SECOND && currentStatus == BEFORE_LANDING){
      fireCharge(6); //Forward Charge - Main
    }
  }else{
    if(previousStatus == BEFORE_APOGEE && currentStatus == BEFORE_LANDING){
      //For non dual deployment both charges will be placed in main charge for redudency
      fireCharge(7);
      fireCharge(6);
    }
  }
  
  //Save data to sd card
  String dataString = String(currentStatus)+","+String(time-launchTime)+","+String(accel)+","+String(temp)+","+String(pres);
  
  char dataChar[dataString.length()+1];
  dataString.toCharArray(dataChar,dataString.length()+1);
  Serial.println(dataChar);
  
  //Transmit to slave arduino which will handle saving
  //and transmitting of telemetry
  if(!USE_TEST_INPUT){
    Wire.beginTransmission(5);
    Wire.write(dataChar);        
    Wire.endTransmission();
  }
}
void setOffsets(){
  voltOffset =  ((double) analogRead(accelPort))*5/1024 - 2.5;
  groundAlt = altAvg;
}
void updateStatus(){
  
  //Check Arming Switch 
  /*if(!armingSwitchEngaged && currentStatus != BEFORE_ARMING){
    currentStatus = BEFORE_ARMING;
  }
  
  if(currentStatus == BEFORE_ARMING){
    if(armingSwitchEngaged){
      currentStatus = BEFORE_LAUNCH;
      
      //Zero out sensors
      setOffsets();
    }
  }else */
  
  if(currentStatus == BEFORE_LAUNCH){
    //If acceleration is greater than
    //expected launch trigger acceleration
    if(accelAvg > LAUNCH_ACCEL){
      launchTime = time;
      currentStatus = DURING_MACH_DELAY;
    }
  }else if(currentStatus == DURING_MACH_DELAY){
    if(time - launchTime > MACH_DELAY){
       currentStatus = BEFORE_APOGEE;
    }
  }else if(currentStatus == BEFORE_APOGEE){
    if(maxAlt < altAvg - groundAlt){
      maxAlt = altAvg - groundAlt;
    }

    Serial.print(time-launchTime); Serial.print(" > "); Serial.println(APOGEE_HIGH_WIN);
    if(((maxAlt - 10 > altAvg - groundAlt) && (time-launchTime > APOGEE_LOW_WIN)) || (time-launchTime > APOGEE_HIGH_WIN)){
      if(USE_DUAL_DEPLOYMENT){
        currentStatus = BEFORE_SECOND;
      }else{
        currentStatus = BEFORE_LANDING;
      }
    }
  }else if(currentStatus == BEFORE_SECOND){
    if((SECOND_TARGET_ALT > altAvg - groundAlt && time-launchTime > SECOND_LOW_WIN) || (time-launchTime > SECOND_HIGH_WIN)){
      currentStatus = BEFORE_LANDING;
    } 
  }else if(currentStatus == BEFORE_LANDING){
    
  }
}

void fireCharge(int port){
  digitalWrite(port, HIGH);   // sets the LED on
  delay(70);                  // waits for a second
  digitalWrite(port, LOW);    // sets the LED off
}

