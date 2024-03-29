#include <QTRSensors.h>

// Analog pin for the short range ir sensor
int FRONT_IR_SENSOR_PIN = 0;

// Analog pin for the left long range ir sensor
int LEFT_IR_SENSOR_PIN = 1;
int LEFT_SHORT_IR_SENSOR_PIN = 3;

// Analog pin for the right long range ir sensor
int RIGHT_IR_SENSOR_PIN = 2;
int RIGHT_SHORT_IR_SENSOR_PIN = 4;

// Analog pin for the back short range sensor
int BACK_SHORT_IR_SENSOR_PIN = 5;

// Pins that control the voltage to the moters
int VOLTAGE_MOTER1 = 11;
int VOLTAGE_MOTER2 = 3;

// Pins that controll the direction of the moters
int DIRECTION_MOTER1 = 13;
int DIRECTION_MOTER2 = 12;

// Default voltage constant
int DEFAULT_VOLTAGE = 42;
int RIGHT_MOTER_OFFSET = 0;

int cyclePeriodMil = 0;

//int rotationPeriodMil = 300;
int rotationPeriodMil = 0;
int preRotationPeriodMil = 400;

// Number of sensors in the QRT Sensor array
int numberOfSensors = 8;

// Create the QTR Sensor object
QTRSensorsRC qtr((unsigned char[]) { 2, 4, 5, 6, 7 ,8 ,9, 10 }, numberOfSensors);
//QTRSensorsRC qtr((unsigned char[]) { 10, 9, 8, 7, 6, 5, 4, 2 }, numberOfSensors);

// Array to store the output from the QTR Sensor read
unsigned int output[8];

// Array to stored the normalised output from the QTR Sensor read
int normalisedOutput[8];

// Stores IR read values
int frontIRReading = 0;
int frontShortIRNormalised = 0;
int leftShortIRNormalised = 0;
int rightShortIRNormalised = 0;
int backShortIRNormalised = 0;

// Left IR sensor values
double leftIRReading = 0;
double lastLeftIRReading = 0;

// Right IR sensor values
double rightIRReading = 0;

//
double lastRightIRReading = 0;

// Stores the current error
int error = 0;

// Stores the last calculated error, used when calculating moter speed
// adjustment
int lastError = 0;

// Stores the last diffrence for quadrent 4
double lastDiffrence = 0;

// 
boolean wasRight = false;

//
boolean ignoreWalls = false;

//
boolean turnedLeft = false;

//
boolean turnedRight = false;

//float KP = 0.9;
//float KD = 2;

//float KP = 1.1;0
//float KD = 2;
float KP = 0.4;
float KD = 2.6;

//
double quad4Intergril = 0;

// Stores the current quadrent the robot is in
int quadrent = 4;

void setup()
{
  Serial.begin(9600);

  boadcastRadio("Connecting      ");

  

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
  
  sendQuadrent();
  
  //setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE+5);
  
  //delay(5000);
  
  //delay(2000);
}

void loop()
{
  // Read from the sensor, unless we are in quadrent 4 because
  // we dont need to worry about it
  readQTRSensor();
  
  // Read from the front IR sensor if we are in any quadrent but the first
  if( quadrent >= 2 ) readFrontShortIRSensor();
  
  // If we are in the 4th quadrent read from the IR sensors
  // on the side of the robot
  if( quadrent >= 3 )
  {
    //
    //readFrontShortIRSensor();
    
    // Read from the left IR sensor
    
    readLeftShortIRSensor();
  
    // Read from the right IR sensor
    
    readRightShortIRSensor();
  }
  
  if( quadrent == 4 )
  {
    readLeftIRSensor();
    
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
  
  //delay(cyclePeriodMil);
}

/*
 * Reads from the QTR Sensor Array and puts values into the array
 * output, then for each value in output the corrosponding value in the
 * normalisedOutput array will be set to 0 if that reading is over 1000
 * ie black and if the value is < 1000 it will be set to 1
 */
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

/*
 * Reads from the front digital short range IR sensor, it then assigns frontIRReading
 * 0 if there is nothing in the way and it assigns 1 if there is a wall in the way
 */
void readFrontShortIRSensor()
{
  // Read the digital sensor as analog, will give a value between 0 and
  // 1024
  //frontIRReading = analogRead(FRONT_IR_SENSOR_PIN);
  
  // Normalise the reading to 0 ( no wall ), 1 ( wall )
  //if( frontIRReading > 200 ) frontIRNormalised = 0;
  //else frontIRNormalised = 1;
  frontShortIRNormalised = readShortIRSensor(FRONT_IR_SENSOR_PIN);
}

void readBackShortIRSensor()
{
  backShortIRNormalised = readShortIRSensor(BACK_SHORT_IR_SENSOR_PIN);
}

void readLeftShortIRSensor()
{
  leftShortIRNormalised = readShortIRSensor(LEFT_SHORT_IR_SENSOR_PIN);
  
}

void readRightShortIRSensor()
{
  rightShortIRNormalised = readShortIRSensor(RIGHT_SHORT_IR_SENSOR_PIN);
}

int readShortIRSensor(int pin)
{
   // Read the digital sensor as analog, will give a value between 0 and
  // 1024
  int reading = analogRead(pin);
  Serial.println(reading);
  
  // Normalise the reading to 0 ( no wall ), 1 ( wall )
  if( reading > 200 ) return 0;
  else return 1;
  
}

/*
 * Retreves the reading for the long range IR sensor on the left
 * of the robot
 */
void readLeftIRSensor()
{
  leftIRReading = readLongRangeIRSensor(LEFT_IR_SENSOR_PIN);
  
  Serial.print("Left IR Reading: ");
  Serial.println(leftIRReading);
}

/*
 * Retreves the reading for the long range IR sensor on the right
 * of the robot
 */
void readRightIRSensor()
{
  rightIRReading = readLongRangeIRSensor(RIGHT_IR_SENSOR_PIN);
  
  Serial.print("Right IR Reading: ");
  Serial.println(rightIRReading);
}

/*
 * Return the value read from the long range IR sensor at
 * the analog pin passed to it. Will return 0 if read value is
 * outside the allowed range
 */
double readLongRangeIRSensor( int pin )
{
  double sum = 0;
  int itter = 10;
  
  for( int i = 0; i < itter; i++ )
  {
  
    // Get the reading from the analog pin
    int reading = analogRead(pin);
  
    // Use the formula on there website to calculate
    // the distance, if its outside the valid range of sensor values
    // just leave the distance as 0 because otherwise we will get incorrect distances
    //double distance = 0;
  
    double distance = 43235.4289561224 * pow(reading, -1.1129015462);
    sum += distance;
    //distance = 43235.4289561224 * pow(reading, -1.1129015462);
  }
  //if( reading >= 80 && reading <= 530 ) 
  //{
  //  distance= 2076.0/(reading - 11);
  //}
  
  //float volts = reading*0.0048828125;   // value from sensor * (5/1024) - if running 3.3.volts then change 5 to 3.3
  //distance = 65*pow(volts, -1.10);
  
  // TESTING: Will remove these print statments for final revision
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.print(". Reading: ");
  //Serial.println(reading);
  
  // TESTING: Return 0 while sensors arnt connected
  //return 0;  
  //return distance;
  return sum/itter;
  
}

/*
 * Calculate the error, where the line is in relation to the sensors.
 * Weights the sensor readings from 0 to 7000, and sums them up, if
 * the sensor at that position is white then weight of that sensor is added to the
 * sum otherwise it remains the same, at the end the sum is devied by number
 * of sensors with white under them, giving an average of ezactly 3500 if the sensors 
 * in the middle are the ones over the line, otherwise it will be smaller
 * towards the left and greater toward the right
 
 * This will give an error of 0 if the middle sensors are over the line other wise
 * negative if robot needs to turn left and positive if robot needs to turn right
 */
void calculateError()
{
  lastError = error;
  
  
  int sum = 0;
  int count = 0;
  
  // If we are in quadrent 2 or 3 we want to ignore the outside sensors
  // because it will make our robot vear off course when we pass a right turn and we
  // ignore it
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
  
  // Calculate the error based on the average
  if( average == -1 ) error = lastError;
  else error = 3500 - average;
  
  // TESTING: Will remvove these print statments later
  Serial.print("Average: ");
  Serial.println(average);
  
  Serial.print("Error: ");
  Serial.println(error);
}

/*
 * Used data read from the various sensors to detect when the
 * robot enteres another quadrent
 */
void determinQuadrent()
{ 
  // Detect quadrent 2, if the robot detects a line on ether the right or
  // left of the robot and wh where just in quadrent 1
  //if( quadrent == 1 && abs(lastError) <= 500 && (canMoveRight() || canMoveLeft())) 
  if( quadrent == 1 && isAllWhite() )
  //if( quadrent == 1 && (canMoveRight() || canMoveLeft()))
  {
    quadrent = 2;
    //setMoterVoltages(0,0);
    
    //DEFAULT_VOLTAGE = 39;
    
    //delay(2000);
    
    sendQuadrent();
  }
  
  // Detect quadrent 3, if the front IR sensor reads that there is a wall and
  // we where just in quadrent 2
  if( quadrent == 2 && frontShortIRNormalised == 1 )
  {
    quadrent = 3;
    //setMoterVoltages(0,0);
    
    //DEFAULT_VOLTAGE = 60;
    
    //delay(2000);
    
    sendQuadrent();
  }
  
  // Detect quadrent 4, if both the long range sensors on the side of the robot read
  // values that are valid and we where just in quadrent 3
  if( quadrent == 3 && ( leftShortIRNormalised == 1 && rightShortIRNormalised == 1 ))
  {
    quadrent = 4;
    
    DEFAULT_VOLTAGE = 35;
    
    setMoterVoltages(0,0);
    delay(2000);
    
    sendQuadrent();
  }
  
  // Detect when finished, if the sensors stop reading valid values and we where in quadrent 3
  //if( quadrent == 4 && ( leftIRReading == 0 && rightIRReading == 0 ))
  //{
  //  quadrent = 5;
    
    //delay(2000);
  //  setMoterVoltages(0,0);
    
  //  sendQuadrent();
  //}
}

/*
 * Print the quadrent to the serial, should be XBee sensor
 */
void sendQuadrent()
{
  if( quadrent == 1 ) boadcastRadio("Quadrent: 1     ");
  else if( quadrent == 2 ) boadcastRadio("Quadrent: 2     ");
  else if( quadrent == 3 ) boadcastRadio("Quadrent: 3     ");
  else if( quadrent == 4 ) boadcastRadio("Quadrent: 4     ");
  else if( quadrent == 5 ) boadcastRadio("Finished!       ");//Serial.println("Finished!");
  //else
  //{
  //  char toSend[] = "Quadrent: " + char(quadrent);
  //  //Serial.print("Quadrent: ");
    //Serial.println(quadrent);
  //}
}

void boadcastRadio(char transmission[])
{
  char frame[34];
  
  int nFrame = 33;
  unsigned int checkSumTotal = 0;
  unsigned int crc = 0;
  
  // delimter
  frame[0] = 0x7e;

  //length
  frame[1] = 0x00;
  frame[2] = 0x1e;

  // API ID
  frame[3] = 0x10;
  frame[4] = 0x01;

  // destination 64 bit address - broadcast
  frame[5] = 0x00;
  frame[6] = 0x13;
  frame[7] = 0xa2;
  frame[8] = 0x00;
  frame[9] = 0x40;
  frame[10] = 0x6a;
  frame[11] = 0x40;
  frame[12] = 0xa4;

  // destination 16 bit address - broadcast
  frame[13] = 0xff;
  frame[14] = 0xfe;
  
  // no. of hops
  frame[15] = 0x00;

  // option
  frame[16] = 0x01;
  
  // data!
  for(int i = 17; i<33; i++)
  {
    frame[i] = transmission[i-17];
  }

  // check sum
  for(int i = 3; i<nFrame; i++)
  {
    checkSumTotal += frame[i];
  }

  checkSumTotal = checkSumTotal & 0xff;
  crc = 0xff - checkSumTotal;
  frame[nFrame] = crc;
  
  nFrame++;

  for(int i=0; i<nFrame; i++)
  {
    Serial.write(frame[i]);
  }
}

/*
 * Determins how to move in the first quadrent
 */
void moveQuadrent1()
{
  // Figure out how much to adjust the voltages by
  int voltageAdjustment = (KP*error + KD*(error - lastError))/100;
  
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
  setMoterVoltages( leftMoterVoltage, rightMoterVoltage + RIGHT_MOTER_OFFSET );
}

/*
 * Determins how to move in quadrent 2
 */
void moveQuadrent2() 
{
  // Priority: Left, Foward, Right, Turn aroud (180)
  
  if( canMoveLeft() ) turnLeft();
  else if( canMoveFoward() ) moveQuadrent1();
  else if( wasRight ) 
  {
    //setMoterVoltages(0, 0);
    
    //digitalWrite(DIRECTION_MOTER1, LOW);
    //digitalWrite(DIRECTION_MOTER2, LOW);
    
    //setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
    
    //while( !canMoveFoward() ) readQTRSensor();
    
    //setMoterVoltages(0, 0);
    
    //digitalWrite(DIRECTION_MOTER1, HIGH);
    //digitalWrite(DIRECTION_MOTER2, HIGH);
    
    turnRight();
  }
  else if( quadrent != 4 ) turnAround();
  
  wasRight = canMoveRight();
  
}

/*
 * Determins how to move in quadrent 3
 */
void moveQuadrent3() 
{
  // If there is a wall turn around
  if( frontShortIRNormalised == 1 ) turnAround();
  
  // Otherwise the movement is the same as in quadrent 2
  else moveQuadrent2();

}

/*
 * Determins how to move in quadrent 4
 */
void moveQuadrent4() 
{ 
  DEFAULT_VOLTAGE = 40;
  
  //float KP4 = 0.7;
  //float KD4 = 1.4;
  
  //float KP4 = 0.5;
  //float KD4 = 0.9;
  
  //float KP4 = 0.5;
  //float KD4 = 6;
  
  float KP4 = 1.2;
  float KI4 = 0.001;
  float KD4 = 5;
  
  //double diffrence = diffrence = leftIRReading - rightIRReading;
  //double diffrence = rightIRReading - lastRightIRReading;
  
  //lastRightIRReading = rightIRReading;
  
  //int voltageAdjustment = ((KP4)*diffrence + KD4*(diffrence - lastDiffrence));
  
  //int rightMoterVoltage = (DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET) + voltageAdjustment;  
  
  //int leftMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
  //if( leftMoterVoltage < 0 ) digitalWrite(DIRECTION_MOTER1, LOW);
  //else digitalWrite(DIRECTION_MOTER1, HIGH);
  
  //if( rightMoterVoltage < 0 ) digitalWrite(DIRECTION_MOTER2, LOW);
  //else digitalWrite(DIRECTION_MOTER2, HIGH);
  
  //Serial.print("Quad 4 Voltage Adjustment ");
  //Serial.println(voltageAdjustment);
   
  // Update the moter voltage pins
  //setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
  
  //Serial.println("Moving Foward");
  //int voltageAdjustment = ((KP4)*diffrence + KD4*(diffrence - lastDiffrence) + KI4*(quad4Intergril));
  
  //int rightMoterVoltage = (DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET) + voltageAdjustment;  
  
  //int leftMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
  //Serial.print("Quad 4 Voltage Adjustment ");
  //Serial.println(voltageAdjustment);
   
  // Update the moter voltage pins
  //setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
  
  //quad4Intergril += diffrence;
  
  if( leftShortIRNormalised = 1 ) turnedLeft = false;
  if( rightShortIRNormalised = 1 ) turnedRight = false;
  
  int diffrence = 0;
  // Diffrence will be negative if you need to move right, and positive if you need to move left
  if( rightShortIRNormalised == 1 && leftShortIRNormalised == 1 ) diffrence = leftIRReading - rightIRReading;
  else diffrence = (rightIRReading - lastRightIRReading);
  
  if( abs( diffrence - lastDiffrence ) > 5 ) diffrence = lastDiffrence;
  
  Serial.println(rightShortIRNormalised);
  delay(1000);
  
  if( rightShortIRNormalised == 0 )
  {
    Serial.println("Turning right");
    delay(2000);
    turnRightQuad4();
    turnedRight = true;
  }
  if( frontShortIRNormalised == 0 )
  {
    Serial.println("Moving Foward");
    int voltageAdjustment = ((KP4)*diffrence + KD4*(diffrence - lastDiffrence));
  
    int rightMoterVoltage = (DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET) + voltageAdjustment;  
  
    int leftMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
    Serial.print("Quad 4 Voltage Adjustment ");
    Serial.println(voltageAdjustment);
   
    // Update the moter voltage pins
    setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
    
  }
  
  else if( leftShortIRNormalised == 0 && !turnedLeft )
  {
    turnLeftQuad4();
    turnedLeft = true;
  }
  
  
  //diffrence = leftIRReading - rightIRReading;
    
  //if( isLine() ) 
  //{
  //  moveQuadrent2();
  //  ignoreWalls = true;
  //}
  //else if( ignoreWalls )
  //{ 
  //  digitalWrite(DIRECTION_MOTER1, HIGH);
  //  digitalWrite(DIRECTION_MOTER2, HIGH);
    
  //  setMoterVoltages(39, 39);
  //  delay( 500 );
  //  setMoterVoltages(0,0);
  //  ignoreWalls = false;
  //}
  
  //if( leftShortIRNormalised == 1 && rightShortIRNormalised == 1 ) turnedLeft = false;
  
  //else if( leftShortIRNormalised == 0 )//&& !ignoreWalls ) 
  //if( leftShortIRNormalised == 0 && !turnedLeft )
  //{
  //  turnedLeft = true;
    //if( ignoreWalls ) ignoreWalls = false;
  //  turnLeftQuad4();
  //}
  //else if( frontShortIRNormalised == 0 )
  //{
  //  int voltageAdjustment = ((KP4)*diffrence + KD4*(diffrence - lastDiffrence));
  
  //  int rightMoterVoltage = (DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET) + voltageAdjustment;  
  
  //  int leftMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
  //  Serial.print("Quad 4 Voltage Adjustment ");
  //  Serial.println(voltageAdjustment);
   
    // Update the moter voltage pins
  //  setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
  //}
  //else if ( rightShortIRNormalised == 0 && frontShortIRNormalised == 1  ) turnRightQuad4();
  //else if( rightShortIRNormalised == 1 && leftShortIRNormalised == 1 && frontShortIRNormalised == 1 ) turnAround();
  //else setMoterVoltages(0,0);
  
  //lastLeftIRReading = leftIRReading;
   
  //if( frontIRNormalised == 1 )
  //{
  //  if ( rightIRReading == 0 && leftIRReading == 0 ) turnAround();
  //  else if( rightIRReading == 0 ) turnRight();
  //  else if( leftIRReading == 0 ) turnLeft();
  //}
  //else
  //{
  //  int voltageAdjustment = ((KP)*diffrence + KD*(diffrence - lastDiffrence));
  
    // Add the adjustment to left because if we want it to turn left 
    // the left moter voltage should be less than the right moter voltage
    // and the error is negative if robot needs to turn left
  //  int leftMoterVoltage = DEFAULT_VOLTAGE + voltageAdjustment;
    
    // TESTING: Remove these print staments later
  //  Serial.print("Quad 4 Voltage Adjustment");
  //  Serial.println(voltageAdjustment);
  
    // Substract the adjustment from the right moter voltage
  //  int rightMoterVoltage = DEFAULT_VOLTAGE - voltageAdjustment;
  
    // Update the moter voltage pins
  //  setMoterVoltages( leftMoterVoltage, rightMoterVoltage );
  //}
  
  
  
  // Keep track of the last error
  lastDiffrence = diffrence;
  
  //
  lastRightIRReading = rightIRReading;
}

/*
 * Returns true if the robot can move foward, ie the sensor isnt just reading
 * black under the sensors
 */
boolean canMoveFoward() 
{
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) return true;
  }
  
  return false;

}

/*
 * Returns true if the robot can move left, ie the sensor is reading white
 * for 4 or more sensors starting from the left
 */
//boolean canMoveLeft() 
boolean canMoveRight()
{
  int leftCount = 0;
  
  // Count from the left
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) leftCount++;
    else break;
  }
  
  if( leftCount >= 4 ) 
  {
    boadcastRadio("Can Turn Right  ");
    return true;
  }
  return false;
}

/*
 * Returns true if the robot can move right, ie the sensor is reading white
 * for 4 or more sensors starting from the right
 */
//boolean canMoveRight()
boolean canMoveLeft()
{
  int rightCount = 0;
  
  // Count from the right
  for( int i = numberOfSensors-1; i >= 0; i-- )
  {
    if( normalisedOutput[i] == 1 ) rightCount++;
    else break;  
  }
  
  if( rightCount >= 4 ) 
  {
    boadcastRadio("Can Turn Left   ");
    return true;
  }
  return false;

}

/*
 * Returns true if the robot is centered on the line, ie if atleast two
 * adjaceant sensors in the array are reading white, how ever this only 
 * looks at the middle 4 sensors of the array
 */
boolean isCenteredOnLine()
{
  readQTRSensor();
  
  if( isAllWhite() ) return false;
  
  //if( normalisedOutput[0] == 1 ) return false;
  //else if( normalisedOutput[numberOfSensors-1] == 1 ) return false;
  
  for( int i = 0; i < 2; i++ )
  {
    if( normalisedOutput[i] == 1 ) return false;
    
  }
  
  for( int i = numberOfSensors-2; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) return false;
    
  }
  
  int count = 0;
  
  for( int i = 2; i < numberOfSensors-2; i++ )
  {
    if(normalisedOutput[i] == 1) count++;
    else if(count == 1) return false;
    
    if(count == 2) 
    {
      boadcastRadio("Is Centered On L");
      return true;
    }
  }
  
  return false;
}

boolean isLine()
{
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 1 ) 
    {
      return true;
    }
  }
  
  return false;
}

boolean isAllWhite()
{
  for( int i = 0; i < numberOfSensors; i++ )
  {
    if( normalisedOutput[i] == 0 ) return false;
    
  }
  
  return true;
  
}

/*
 * Turns the robot left by aprox 90 degrees
 */
void turnLeft() 
{
  //if( quadrent == 4 ) backPreRotationMove();
  
  //boadcastRadio("Turning Left    ");
  Serial.println("Turning Left");
  if(quadrent != 4) preRotationMove();
  
  //digitalWrite(DIRECTION_MOTER1, HIGH);
  //digitalWrite(DIRECTION_MOTER2, LOW);
  
  digitalWrite(DIRECTION_MOTER1, LOW);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
  delay(preRotationPeriodMil);
  
  while(!isCenteredOnLine())
  {
    //delay(rotationPeriodMil);
  }
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void turnLeftQuad4()
{
  
  preRotationMoveQuad4();
  //readBackShortIRSensor();
    
  digitalWrite(DIRECTION_MOTER1, LOW);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );  
  
  //readBackShortIRSensor();
    
  //while( backShortIRNormalised == 0 )
  //{
  //  readBackShortIRSensor();
  //}
  
  delay(1200);
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  
  
  
}

/*
 * Turns the robot right by aprox 90 degrees
 */
void turnRight() 
{
  //setMoterVoltages(0,0);
  //delay( 4000);
  
  //if( quadrent == 4 ) 
  //else preRotationMove();
  
  //setMoterVoltages(0,0);
  //delay( 4000 );
  
  
  
  //boadcastRadio("Turning Right    ");
  Serial.println("Turning Right");
  //preRotationMove();
  
  //double origonalLeftReading = leftIRReading;
  
  //digitalWrite(DIRECTION_MOTER1, LOW);
  //digitalWrite(DIRECTION_MOTER2, HIGH);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, LOW);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
  delay(preRotationPeriodMil);
  
  while(!isCenteredOnLine())
  {
    //delay(rotationPeriodMil);
  }
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

void turnRightQuad4()
{
  preRotationMoveQuad4();
  backPreRotationMove();
  
  //readLeftIRSensor();
  //double startingRead = leftIRReading;
  
  //setMoterVoltages(0,0);
  //setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE);
  //delay(300);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, LOW);
  
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
  delay(300);
  
  //readBackShortIRSensor();
  //readLeftShortIRSensor();
  //readRightShortIRSensor();
  //readFrontShortIRSensor();
  //readLeftIRSensor();
   
  //abs(leftIRReading - startingRead) > 0.6 &&
  
  //while(  backShortIRNormalised == 1 && leftShortIRNormalised == 1 )
  //{
  //  readBackShortIRSensor();
  //  readLeftShortIRSensor();
  //  delay(100);
    //readLeftIRSensor();
  //}
  
  //delay(350);
  
  //delay(1100);
  
  while( backShortIRNormalised == 0 )
  {
    readBackShortIRSensor();
    //delay(100);
  }
    
  //while( backShortIRNormalised == 0 || leftShortIRNormalised == 0 || rightShortIRNormalised == 1 || frontShortIRNormalised == 1 )
  //{
  //  readBackShortIRSensor();
  //  readLeftShortIRSensor();
  //  readRightShortIRSensor();
  //  readFrontShortIRSensor();
  //}
  
  setMoterVoltages(0, 0);
  
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
}

/*
 * Turns the robot by aprox 180 degrees
 */
void turnAround() 
{
  //setMoterVoltages(0,0);
  //delay( 4000);
  
  //backPreRotationMove();
  
  //setMoterVoltages(0,0);
  //delay( 4000 );
  
  //backPreRotationMove();
  
  //boadcastRadio("Turning Around  ");
  Serial.println("Turning Around");
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, LOW);
  
  if( quadrent == 2 || quadrent == 3 )
  {
    
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
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
    
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
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

/*
 * Offsets the robot for any left or right rotation
 */
void preRotationMove()
{
  // If we are in quadrent 4 we dont need to move 
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
    
  //while( canMoveLeft() || canMoveRight() ) readQTRSensor();
    
  delay(250);
    
  setMoterVoltages(0, 0);
  //delay(350);
  //delay(100);
  //delay(200);
}

void preRotationMoveQuad4()
{
  // If we are in quadrent 4 we dont need to move 
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
    
  //while( canMoveLeft() || canMoveRight() ) readQTRSensor();
    
  while( frontShortIRNormalised == 0 ) readFrontShortIRSensor();  
  //delay(500);
    
  setMoterVoltages(0, 0);
  //delay(350);
  //delay(100);
  //delay(200);
}

void backPreRotationMove()
{
  setMoterVoltages(0, 0);
    
  digitalWrite(DIRECTION_MOTER1, LOW);
  digitalWrite(DIRECTION_MOTER2, LOW);
    
  setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
    
  delay(150);
    
  setMoterVoltages(0, 0);
    
  digitalWrite(DIRECTION_MOTER1, HIGH);
  digitalWrite(DIRECTION_MOTER2, HIGH);
  
  if( quadrent != 4 )
  {
    setMoterVoltages(0, 0);
    
    digitalWrite(DIRECTION_MOTER1, LOW);
    digitalWrite(DIRECTION_MOTER2, LOW);
    
    setMoterVoltages(DEFAULT_VOLTAGE, DEFAULT_VOLTAGE + RIGHT_MOTER_OFFSET );
    
    delay(300);
    
    setMoterVoltages(0, 0);
    
    digitalWrite(DIRECTION_MOTER1, HIGH);
    digitalWrite(DIRECTION_MOTER2, HIGH);
   
  } 
}

/*
 * Update the moter voltages to the ones passed to the function
 */
void setMoterVoltages( int newLeftVoltage, int newRightVoltage )
{
  // Make sure the voltages arnt negative  
  if( newLeftVoltage < 0 ) newLeftVoltage = 0;
  if( newRightVoltage < 0 ) newRightVoltage = 0;
   
  // Write the new voltages to the voltage pins
  analogWrite(VOLTAGE_MOTER1, newLeftVoltage);
  analogWrite(VOLTAGE_MOTER2, newRightVoltage);
}

