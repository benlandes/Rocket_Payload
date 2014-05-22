double voltOffset1;
double voltOffset2;
void setup() {
  // put your setup code here, to run once:
delay(1000);
voltOffset1 = ((double) analogRead(0))*5/1024 - 2.5;
voltOffset2 = ((double) analogRead(2))*5/1024 - 2.5;
}

 //Zero out acceleration reading
void loop() {
  double voltageAccel = ((double) analogRead(0))*5/1024 - voltOffset1;
  double g = (voltageAccel-2.5)/0.038;
  double a = g*32.2;
 
 Serial.println(voltOffset2);
  Serial.print(a);
  Serial.println(" (ft/s^2)");
  
  delay(500);
}
