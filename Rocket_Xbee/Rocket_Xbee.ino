void setup() {
  Serial1.begin(9600); 
  while (!Serial1)  {
  }    
}

void loop()
{
  Serial1.println("Connected");
 Serial1.println(millis());
 delay(1000);
}
