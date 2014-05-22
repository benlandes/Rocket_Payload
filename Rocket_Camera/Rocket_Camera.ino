void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT); 
  digitalWrite(6, LOW); 
  
}

void loop() { 
  // put your main code here, to run repeatedly:
  digitalWrite(6, HIGH); 
  delay(300);
  digitalWrite(6, LOW); 
  delay(300);
  digitalWrite(6, HIGH); 
  delay(300);
  Serial.println("Camera On");
  digitalWrite(6, LOW); 
  delay(5000);
}
void turnOnOffCamera(){
  
}
