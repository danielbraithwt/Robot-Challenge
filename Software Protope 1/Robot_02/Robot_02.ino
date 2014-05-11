#include <QTRSensors.h>

// Pins that control the voltage to the moters
int VOLTAGE_MOTER1 = 3;
int VOLTAGE_MOTER2 = 11;

// Pins that controll the direction of the moters
int DIRECTION_MOTER1 = 12;
int DIRECTION_MOTER2 = 13;

// Default voltage constant
int DEFAULT_VOLTAGE = 100;

// Number of sensors in the QRT Sensor array
int numberOfSensors = 8;

// Create the QTR Sensor object
QTRSensorsRC qtr((unsigned char[]) { 2, 4, 5, 6, 7 ,8 ,9, 10 }, numberOfSensors);

// Array to store the output from the QTR Sensor read
unsigned int output[8];

// Array to stored the normalised output from the QTR Sensor read
int normalisedOutput[8];

// Stores the current error
int error = 0;

// Stores the current quadrent the robot is in
int quadrent = 1;

void setup()
{
  Serial.begin(9600);

  // Setup the moter controll pins
  pinMode(VOLTAGE_MOTER1, OUTPUT);
  pinMode(VOLTAGE_MOTER2, OUTPUT);
  pinMode(DIRECTION_MOTER1, OUTPUT);
  pinMode(DIRECTION_MOTER2, OUTPUT);
  
  // Set robot to stationary
  setMoterVoltages(0, 0);
  
  // Set both moters to foward
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void loop()
{
  // Read from the sensor
  readQTRSensor();
  
  // Calcualte the error of the robot
  calculateError();
  
  // Update the quadrent variable if the quadrent is changing
  determinQuadrent();
  
  // Move based on what quadrent the robot is in
  if( quadrent == 1 ) moveQuadrent1();
  else if( quadrent == 2 ) moveQuadrent2();
  else if( quadrent == 3 ) moveQuadrent3();
  else if( quadrent == 4 ) moveQuadrent4();
  
}

void readQTRSensor()
{
  // Read from the sensor into the array
  qtr.read(output);
  
  // Normalise the output from the sensor, 1 being white
  // 0 being black
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( output[i] > 1000 ) normalisedOutput[i] = 0;
    else normalisedOutput[i] = 1;
    
    // TESTING: Print this out to see what is happerning
    Serial.print(normalisedOutput[i]);
    Serial.print(" ");
  }
  
  Serial.println();
}

void calculateError()
{
  // Calculate the error, where the line is in relation to the sensors.
  // Weights the sensor readings from 0 to 7000, and sums them up, if
  // the sensor at that position is white then something is added to the
  // sum otherwise it remains the same, at the end the sum is devied by number
  // of sensors with white under them, giving an average of ezactly 3500 if the sensors 
  // in the middle are the ones over the line, otherwise it will be smaller
  // towards the left and greater toward the right
  int sum = 0;
  int count = 0;
  
  for( int i = 0; i < numberOfSensors; i++ )
  {
    sum += i * ( 1000 * normalisedOutput[i] );
    count += normalisedOutput[i];
  }
  
  // Calulate the average position of the line
  int average = sum/count;
  
  // Calculate the error, sinse if the line is in the middle of the sensors you get
  // an average of 3500 this means with the equason below you will get an error of 0
  // if the sensor is in the middle otherwise it will be negative if the robot needs to move left 
  // and it will be positive if the robot needs to move right
  error = 3500 - average;
  
  // TESTING: Will remvove these print statments later
  Serial.print("Error: ");
  Serial.println(error);
}

void determinQuadrent()
{
  // TESTING: At the moment it just does quadrent 1 but later we will determin how to 
  // tell between the diffrent quadrents
  quadrent = 1;
}

void moveQuadrent1()
{
  // Figure out how much to adjust the voltages by
  // NOTE: There is probberly a better way to do this
  int voltageAdjustment = error/100;
  
  // Add the adjustment to left because if we want it to turn left 
  // the left moter voltage should be less than the right moter voltage
  // and the error is negative if robot needs to turn left
  int leftMoterVoltage = DEFAULT_VOLTAGE + voltageAdjustment;
  
  // Substract the adjustment from the right moter voltage
  int rightMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
  // TESTING: Will remvove these print statments later
  Serial.print("Voltage Adjustment: ");
  Serial.println(voltageAdjustment);
  
  Serial.print("Left Moter: ");
  Serial.print(leftMoterVoltage);
  Serial.print(". Right Moter: ");
  Serial.println(rightMoterVoltage);
  
  // Update the moter voltage pins
  setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
}

void moveQuadrent2() {}
void moveQuadrent3() {}
void moveQuadrent4() {}

void setMoterVoltages( int newLeftVoltage, int newRightVoltage )
{
  // Write the new voltages to the voltage pins
  analogWrite(VOLTAGE_MOTER1, newLeftVoltage);
  analogWrite(VOLTAGE_MOTER2, newRightVoltage);
}
