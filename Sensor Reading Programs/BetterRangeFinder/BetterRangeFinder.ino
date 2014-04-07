// Better Range Finder //
// Combines the readings from long and short range sensors

int LONG_RANGE_IN = 0;
int SHORT_RANGE_IN = 13;

void setup()
{
 Serial.begin(9600); 
 
 // Setup the digital pin
 pinMode(SHORT_RANGE_IN, INPUT);
}

void loop()
{
  // Get long range reading
  int long_reading = analogRead(LONG_RANGE_IN);
  double long_distance = -1;
  if( long_reading >= 80 && long_reading <= 530 )
  {
    long_distance = 2076.0/(long_reading - 11);
  }
  
  // Get short range reading
  int short_reading = digitalRead(SHORT_RANGE_IN);
  
  double processedReading = processDistanceReadings((double)short_reading, long_distance);
  
  Serial.print("Distance: ");
  Serial.println(processedReading);
 
  delay(100); 
}

double processDistanceReadings(double s, double l)
{
  if( s == 0 ) return 4.0;
  else if( s == 1 && l == -1 ) return 30.0;
  
  return l; 
}
