#include <QTRSensors.h>

// Analog pin for the short range ir sensor
int FRONT_IR_SENSOR_PIN = 0;

// Analog pin for the left long range ir sensor
int LEFT_IR_SENSOR_PIN = 1;

// Analog pin for the right long range ir sensor
int RIGHT_IR_SENSOR_PIN = 2;

// Pins that control the voltage to the moters
int VOLTAGE_MOTER1 = 3;
int VOLTAGE_MOTER2 = 11;

// Pins that controll the direction of the moters
int DIRECTION_MOTER1 = 12;
int DIRECTION_MOTER2 = 13;

// Default voltage constant
int DEFAULT_VOLTAGE = 34;

int cyclePeriodMil = 20;

int rotationPeriodMil = 200;
int preRotationPeriodMil = 500;

// Number of sensors in the QRT Sensor array
int numberOfSensors = 8;

// Create the QTR Sensor object
//QTRSensorsRC qtr((unsigned char[]) { 2, 4, 5, 6, 7 ,8 ,9, 10 }, numberOfSensors);
QTRSensorsRC qtr((unsigned char[]) { 10, 9, 8, 7, 6, 5, 4, 2 }, numberOfSensors);

// Array to store the output from the QTR Sensor read
unsigned int output[8];

// Array to stored the normalised output from the QTR Sensor read
int normalisedOutput[8];

// Stores IR read values
int frontIRReading = 0;
int frontIRNormalised = 0;

// Left IR sensor values
double leftIRReading = 0;

// Right IR sensor values
double rightIRReading = 0;

// Stores the current error
int error = 0;

int lastError = 0;

//float KP = 0.9;
//float KD = 2;

float KP = 1.1;
float KD = 2;



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
  
  // Read from the front IR sensor
  if( quadrent >= 2 ) readFrontIRSensor();
  
  if( quadrent >= 3 )
  {
    // Read from the left IR sensor
    readLeftIRSensor();
  
    // Read from the right IR sensor
    readRightIRSensor();
  }
  
  // Calcualte the error of the robot
  calculateError();
  
  // Update the quadrent variable if the quadrent is changing
  determinQuadrent();
  
  // Move based on what quadrent the robot is in
  if( quadrent == 1 ) moveQuadrent1();
  else if( quadrent == 2 ) moveQuadrent2();
  else if( quadrent == 3 ) moveQuadrent3();
  else if( quadrent == 4 ) moveQuadrent4();
  
  delay(cyclePeriodMil);
  //delay(500);
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

void readFrontIRSensor()
{
  frontIRReading = analogRead(FRONT_IR_SENSOR_PIN);
  
  if( frontIRReading > 200 ) frontIRNormalised = 0;
  else frontIRNormalised = 1;
  
  Serial.print("Front Sensor Reading: ");
  Serial.println(frontIRReading);
}

void readLeftIRSensor()
{
  leftIRReading = readLongRangeIRSensor(LEFT_IR_SENSOR_PIN);
}

void readRightIRSensor()
{
  rightIRReading = readLongRangeIRSensor(RIGHT_IR_SENSOR_PIN);
  
}

double readLongRangeIRSensor( int pin )
{
  // Get the reading and convert to a distance
  int reading = analogRead(pin);
  double distance = 0;
  if( reading >= 80 && reading <=530 ) 
  {
    distance= 2076.0/(reading - 11);
  }
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(". Reading: ");
  Serial.println(reading);
  
  return reading;
  
}

void calculateError()
{
  lastError = error;
  
  // Calculate the error, where the line is in relation to the sensors.
  // Weights the sensor readings from 0 to 7000, and sums them up, if
  // the sensor at that position is white then something is added to the
  // sum otherwise it remains the same, at the end the sum is devied by number
  // of sensors with white under them, giving an average of ezactly 3500 if the sensors 
  // in the middle are the ones over the line, otherwise it will be smaller
  // towards the left and greater toward the right
  int sum = 0;
  int count = 0;
  
  if( quadrent >= 2 )
  {
    for( int i = 2; i < numberOfSensors-2; i++ )
    {
      sum += i * ( 1000 * normalisedOutput[i] );
      count += normalisedOutput[i];
    }
    
  }
  else
  {
    for( int i = 0; i < numberOfSensors; i++ )
    {
      sum += i * ( 1000 * normalisedOutput[i] );
      count += normalisedOutput[i];
    }
  }
  
  // Calulate the average position of the line
  int average = sum/count;
  
  // Calculate the error, sinse if the line is in the middle of the sensors you get
  // an average of 3500 this means with the equason below you will get an error of 0
  // if the sensor is in the middle otherwise it will be negative if the robot needs to move left 
  // and it will be positive if the robot needs to move right
  if( average == -1 ) error = lastError;
  else error = 3500 - average;
  
  Serial.print("Average: ");
  Serial.println(average);
  
  // TESTING: Will remvove these print statments later
  Serial.print("Error: ");
  Serial.println(error);
}

void determinQuadrent()
{
  // TESTING: At the moment it just does quadrent 1 but later we will determin how to 
  // tell between the diffrent quadrents
  //quadrent = 1;

  
  if( quadrent == 1 && abs(lastError) <= 500 && (canMoveRight() || canMoveLeft())) 
  {
    quadrent = 2;
    setMoterVoltages(0,0);
    
    DEFAULT_VOLTAGE = 36;
    
    delay(2000);
  }
  
  // Detect quadrent 3
  if( quadrent == 2 && frontIRNormalised == 1 )
  {
    quadrent = 3;
    setMoterVoltages(0,0);
    
    //DEFAULT_VOLTAGE = 60;
    
    delay(2000);
  }
  
  // Detect quadrent 4
  if( quadrent == 3 && ( leftIRReading != -1 && rightIRReading != -1 ))
  {
    quadrent = 4;
    
    delay(2000);
  }
}

void moveQuadrent1()
{
  // Figure out how much to adjust the voltages by
  // NOTE: There is probberly a better way to do this
  int voltageAdjustment = (KP*error + KD*(error - lastError))/100;//error*errorScale;
  
  //if( abs(voltageAdjustment) <= 6 ) voltageAdjustment = 0;
  
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

void moveQuadrent2() 
{
  // Priority: Left, Foward, Right, Turn aroud (180)
  
  //setMoterVoltages(0,0);
  
  if( canMoveLeft() ) turnLeft();
  else if( canMoveFoward() ) moveQuadrent1();//moveStraight();
  else if( canMoveRight() ) turnRight();
  else turnAround();
  
}

void moveQuadrent3() 
{
  if( frontIRNormalised == 1 ) turnAround();
  else moveQuadrent2();

}
void moveQuadrent4() 
{
  // Diffrence will be negative if you need to move right, and positive if you need to move left
  double diffrence = leftIRReading - rightIRReading;
  
  if( frontIRNormalised == 1 )
  {
    if ( rightIRReading == 0 && leftIRReading == 0 ) turnAround();
    else if( rightIRReading == 0 ) turnRight();
    else if( leftIRReading == 0 ) turnLeft();
  }
  else
  {
    int voltageAdjustment = ((KP)*error + KD*(error - lastError));
  
    // Add the adjustment to left because if we want it to turn left 
    // the left moter voltage should be less than the right moter voltage
    // and the error is negative if robot needs to turn left
    int leftMoterVoltage = DEFAULT_VOLTAGE + voltageAdjustment;
  
    // Substract the adjustment from the right moter voltage
    int rightMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
    // Update the moter voltage pins
    setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
  }

}

boolean canMoveFoward() 
{
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) return true;
  }
  
  return false;

}

boolean canMoveLeft() 
{
  int leftCount = 0;
  
  // Count from the left
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) leftCount++;
    else break;
  }
  
  if( leftCount >= 4 ) return true;
  return false;
}

boolean canMoveRight() 
{
  int rightCount = 0;
  
  // Count from the right
  for( int i = numberOfSensors-1; i >= 0; i-- )
  {
    if( normalisedOutput[i] == 1 ) rightCount++;
    else break;  
  }
  
  if( rightCount >= 4 ) return true;
  return false;

}

boolean isCenteredOnLine()
{
  readQTRSensor();
  
  int count = 0;
  
  for( int i = 2; i < numberOfSensors-2; i++ )
  {
    if(normalisedOutput[i] == 1) count++;
    else if(count == 1) return false;
    
    if(count == 2) 
    {
      Serial.println("Is centered on line");
      //delay(2000);
      return true;
    }
  }
  
  return false;
  
  //readQTRSensor();
  //calculateError();
  
  //return ( error == 0 );
}

void turnLeft() 
{
  preRotationMove();
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, LOW);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
  delay(preRotationPeriodMil);
  
  if( quadrent == 2 || quadrent == 3 )
  {
    while(!isCenteredOnLine())
    {
      //delay(rotationPeriodMil);
    }
  }
  else
  {
    while( leftIRReading != 0 );
    
  }
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void turnRight() 
{
  preRotationMove();
  
  digitalWrite(DIRECTION_MOTER1, LOW);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
  delay(preRotationPeriodMil);
  
  if( quadrent == 2 || quadrent == 3 )
  {
    while(!isCenteredOnLine())
    {
      //delay(rotationPeriodMil);
    }
  }
  else 
  {
    while( rightIRReading != 0 );
    
  }
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void turnAround() 
{
  digitalWrite(DIRECTION_MOTER1, LOW);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  if( quadrent == 2 || quadrent == 3 )
  {
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
    delay(preRotationPeriodMil);
    
    while(!isCenteredOnLine())
    {
    //delay(rotationPeriodMil);
    }
  }
  else
  {
    double preRotationLeft = leftIRReading;
    double preRotationRight = rightIRReading;
    
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
    delay(preRotationPeriodMil);
    
    readLeftIRSensor();
    readRightIRSensor();
    
    // Sinse we are turning around we want to turn untill the origonal left value is
    // close to the new right value and visa versa
    while(abs(preRotationLeft - rightIRReading) > 0.5 && abs(preRotationRight - leftIRReading) > 0.5)
    {
      readLeftIRSensor();
      readRightIRSensor();
    }
    
  }
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void moveStraight()
{
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
}

void preRotationMove()
{
  // If we are in quadrent 4 we dont need to move 
  if( quadrent != 4 )
  {
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
    delay(200);
  }
}

void setMoterVoltages( int newLeftVoltage, int newRightVoltage )
{
   //if( newLeftVoltage < newRightVoltage ) Serial.println("LEFT");
   //else if( newLeftVoltage > newRightVoltage ) Serial.println("RIGHT"); 
   
   if( newLeftVoltage < 0 ) newLeftVoltage = 0;
   if( newRightVoltage < 0 ) newRightVoltage = 0;
   
  // Write the new voltages to the voltage pins
  analogWrite(VOLTAGE_MOTER1, newLeftVoltage);
  analogWrite(VOLTAGE_MOTER2, newRightVoltage);
}

