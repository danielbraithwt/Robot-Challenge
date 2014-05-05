#include <QTRSensors.h>

// Number of sensors
int numOfSensors = 8;
// Sensor Pins ( Digital Pins )
int pins[] = { 13, 12, 11, 10, 9, 8, 7, 6, 5 };
// Create array for output from sensors
unsigned int output[8];

// Create the sensor object
QTRSensorsRC qtr((unsigned char[]) { 2, 4, 5, 6, 7 ,8 ,9, 10 }, numOfSensors);

void setup()
{
  Serial.begin(9600);
  
  //Serial.println("Starting callabration in 3 seconds");
  //delay(3000);
  
  // Calabrate the sensors
  //Serial.println("Callabrating.... Expose all your sensors to the diffrent extremes");
  
  //for( int i = 0; i < i < 250; i++ )
  //{
  //  qtr.calibrate();
  //  delay(20);
  //}
  
  //Serial.println("Callbration finished");
  
}

void loop()
{
  qtr.read(output);
  
  for( int i = 0; i < 8; i++ )
  {
   Serial.print(output[i]);
   Serial.print(" ");
   
  }
  
  Serial.println();
  
  // Get the position of the line
  //int pos = qtr.readLine(output, 1);
  
  
  // Calculate the error
  //int error = pos - 5000;
  
  //Serial.print("Error: ");
  //Serial.println(error);
  
  delay(100);
}
