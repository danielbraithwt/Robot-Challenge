/////////////////////////////////////////////
// Long range IR sensor:                   //
//                                         //
// Distance (cm) = 2076/(SensorValue - 11) //
/////////////////////////////////////////////

int SENSOR_IN = 0;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  // Get the reading and convert to a distance
  int reading = analogRead(SENSOR_IN);
  double distance = 0;
  if( reading >= 80 && reading <=530 ) 
  {
    distance= 2076.0/(reading - 11);
  }
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(". Reading: ");
  Serial.println(reading);
  
  delay(100);
}
  
  
