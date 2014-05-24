

#include <SFE_BMP180.h>
#include <Wire.h>

//For Unit Testing
#define USE_TEST_INPUT true

//Program Input
#define USE_DUAL_DEPLOYMENT true
#define MAIN_TARGET_ALT 2000 //ft
#define LAUNCH_ACCEL 4 //G

//Safety Timers
#define MACH_DELAY 9 //sec
#define DROGUE_LOW_WIN 15 //Sec
#define DROGUE_HIGH_WIN 60 //Sec
#define MAIN_LOW_WIN 100 //Sec
#define MAIN_HIGH_WIN 300 //Sec

//Ports
const int accelPort = 0; //Analog

//Calibration
#define SEAP 1019.0 //mbar

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
float voltOffset, groundAlt; //V, ft

void setup() {
  
  //Ejection Pins
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
  
  //For communicating with slave xbee
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event

  pressure.begin();

}



double accel, alt, temp, pres;
bool armingSwitchEngaged = false;
double launchTime, time;
float maxAlt = 0;
uint32_t timer = millis();

//Receiving data from slave
void receiveEvent(int howMany)
{
  char line[howMany+1];
  line[howMany] = '\0';
  if(USE_TEST_INPUT){
    for(int i = 0; i < howMany; i++){
      line[i] = Wire.read();
    }
    //Serial.println(line); 
    time = atof(strtok(line,","));
    alt = atof(strtok(NULL,","));
    accel = atof(strtok(NULL,","));
    temp = atof(strtok(NULL,","));
    armingSwitchEngaged = atoi(strtok(NULL,","));
    
    process();
    
  }else{
    while(0 < Wire.available()) // loop through all but the last
    {
      char c = Wire.read(); // receive byte as a character
      Serial.print(c);         // print the character
    }
    Serial.println();         // Newline
  }
  
}
void loop() {
  
  //When unit testing data will be read from sd card instead
  //of from sensors
  if(USE_TEST_INPUT == false){
    //// Gather Data ////
    
    //Accelerometer
    double voltageAccel = ((double) analogRead(accelPort))*5/1024 - voltOffset;
    accel = (voltageAccel-2.5)/0.008*9.81; //m/s^2
    
    Serial.println("h2");
   
    //Getting temperature increases acuracy of pressure calculation
    char status = pressure.startTemperature();
    if (status != 0)
    {
      Serial.println("h4");
      // Wait for the measurement to complete:
      delay(status);
      pressure.getTemperature(temp);
      Serial.println("h5");
      status = pressure.startPressure(3); //3 - High Resolution
      if(status != 0){
        Serial.println("h6");
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
  }
  
}
void process(){
  Serial.println("h7");
  //// Update Status ////
  int previousStatus = currentStatus;
  
  updateStatus();
  
  //Check if ejection charges need to be fired
  if(USE_DUAL_DEPLOYMENT){
    if(previousStatus == BEFORE_APOGEE && currentStatus == BEFORE_MAIN){
      fireCharge(7); //Aft Charge - Drouge 
    }else if(previousStatus == BEFORE_MAIN && currentStatus == BEFORE_LANDING){
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
  String dataString = String(currentStatus)+","+String(millis())+","+String(accel)+","+String(temp)+","+String(pres)+","+String(alt);
  
  char dataChar[dataString.length()+1];
  dataString.toCharArray(dataChar,dataString.length()+1);
  Serial.println(dataChar);
  
  //Transmit to slave arduino which will handle saving
  //and transmitting of telemetry
  Wire.beginTransmission(5);
  Wire.write(dataChar);        
  Wire.endTransmission();
  
  delay(250);
  
}
void setOffsets(){
  voltOffset =  ((double) analogRead(accelPort))*5/1024 - 2.5;
  groundAlt = alt;
}
void updateStatus(){
  
  //Check Arming Switch 
  if(!armingSwitchEngaged && currentStatus != BEFORE_ARMING){
    currentStatus = BEFORE_ARMING;
  }
  
  if(currentStatus == BEFORE_ARMING){
    if(armingSwitchEngaged){
      currentStatus = BEFORE_LAUNCH;
      
      //Zero out sensors
      setOffsets();
    }
  }else if(currentStatus == BEFORE_LAUNCH){
    //If acceleration is greater than
    //expected launch trigger acceleration
    if(accel > LAUNCH_ACCEL){
      launchTime = time;
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
      if(USE_DUAL_DEPLOYMENT){
        currentStatus = BEFORE_MAIN;
      }else{
        currentStatus == BEFORE_LANDING;
      }
    }
  }else if(currentStatus == BEFORE_MAIN){
    if(MAIN_TARGET_ALT -30 > alt - groundAlt){
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

