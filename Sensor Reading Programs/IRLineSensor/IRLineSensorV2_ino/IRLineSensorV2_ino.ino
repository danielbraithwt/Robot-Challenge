/*
   Code for reading raw values from line sensor. 
   Sensor should be about 3 mm away from the surface and 
   not touch it.
   Sensor wires are connected to Arduino pins 2,4,5,6,7,8,9,10
*/


// libraries
#include <QTRSensors.h>
#define NUM_SENSORS	8   // constant value

// initialize sensor class/variable
QTRSensorsRC qtr((unsigned char[]) {2,4,5,6,7,8,9,10}, NUM_SENSORS);
// array to store signals from all 8 sensors
unsigned int sensorValues[NUM_SENSORS];

void setup()
{
   Serial.begin(9600);
}

void loop()
{
  int i;
  
  qtr.read(sensorValues,QTR_EMITTERS_ON);
  for ( i = 0 ; i < NUM_SENSORS ;i++){
    Serial.print(sensorValues[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  delay(1000);
  
}
