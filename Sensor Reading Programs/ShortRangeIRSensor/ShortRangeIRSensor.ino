//////////////////////////////
// Short range IR sensor    //
//                          //
// Outputs 0 if object is   //
// within 5 cm of something //
// outherwise 1 is outputed //
//////////////////////////////

int SENSOR_IN = 13;

void setup()
{
  Serial.begin(9600);
  
  // Setup the input pin
  pinMode( SENSOR_IN, INPUT ); 
}

void loop()
{
  int reading = digitalRead(SENSOR_IN);
  
  Serial.println(reading);
}
