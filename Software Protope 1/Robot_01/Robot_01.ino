#include <QTRSensors.h>

int pwm_a = 3;  //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 11;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int dir_a = 12;  //dir control for motor outputs 1 and 2 is on digital pin 12
int dir_b = 13;  //dir control for motor outputs 3 and 4 is on digital pin 13
int move_pulse = 100;

float lastError = 0;
float KP = 0.003;
float KD = 0.001;

// Number of sensors
int numOfSensors = 8;
// Sensor Pins ( Digital Pins )
int pins[] = { 13, 12, 11, 10, 9, 8, 7, 6, 5 };
// Create array for output from sensors
unsigned int output[8];
unsigned int normOutput[8];
unsigned int error;
unsigned int quadrent = 1;

// Create the sensor object
QTRSensorsRC qtr((unsigned char[]) { 2, 4, 5, 6, 7 ,8 ,9, 10 }, numOfSensors);

void setup()
{ 
  // initializ motor pins  
  pinMode(pwm_a, OUTPUT);  //Set control pins to be outputs
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);
  analogWrite(pwm_a, 0);  // set motor voltages 0
  analogWrite(pwm_b, 0);
  
  Serial.begin(9600);     // for communacations with computer
}

void loop()
{
  // Collect and process sensor data
  readFromLineSensor();
  calculateError();
  
  // Determin the what quadrent the robot is in
  detectQuadrent();
  
  // Move robot based on what quadrent its in
  if( quadrent == 1 ) moveQuadrent1();
  else if( quadrent == 2 ) moveQuadrent2();
  else if( quadrent == 3 ) moveQuadrent3();
  else if( quadrent == 4 ) moveQuadrent4();
}

/**
* Reads data from the QTR Line Sensor into the 
* output arrays
**/
void readFromLineSensor()
{
  // Read the data values from the QTR Line Sensor
  qtr.read(output);
  
  // TESTING: Print out sensor read values
  for( int i = 0; i < numOfSensors; i++ )
  {
   Serial.print(output[i]);
   Serial.print(" ");
   
  }
  Serial.println();
  
  // Normalise the output into 1 ( white ) and 0 ( black )
  for( int i = 0; i < numOfSensors; i++ )
  {
    if( output[i] > 1500 ) normOutput[i] = 0;
    else normOutput[i] = 1;
  }
  
  // TESTING: Print out normalised values
  for( int i = 0; i < numOfSensors; i++ )
  {
   Serial.print(normOutput[i]);
   Serial.print(" ");
   
  }
  
  Serial.println();
}

/**
* Uses the read sensor values to calculate an error
* value
**/
void calculateError()
{
  // Sum up the sensor values 
  int average = 0;
  int count = 0;
  
  for( int i = 0; i < numOfSensors; i++ )
  {
    average += ( i * 1000 ) * normOutput[i];
    count += normOutput[i];
  } 
  
  // Calculate the average
  average = average / count;
  
  // Calculate the error, ie the diffrence between the middle and the average
  error = 3500 - average;
  
  Serial.print("[*] Error: ");
  Serial.println(error);
}

/**
* Determins what the quadrent the robot is in
**/
void detectQuadrent()
{
  // If one side of line sensor is reading just 1's
  if( abs(error) == 3500 ) quadrent = 2;
  
  // Account for other quadrents here
  
  // Sends the current quadrent to the base station
  sendQuadrent();
}

/**
*
**/
void moveQuadrent1()
{
  //int delta = KP * (error/100) + KD * ( (error/100) - lastError );
  int delta = KP*error + KD*(error - lastError);
  lastError = error/100 ;
  
  
  Serial.print(". ");
  Serial.print("[*] Delta: ");
  Serial.println(delta);
  
  // If error is less than 0 then that means that the line towards the right of the sensors
  
  int left = 100;
  int right = 100;
  
  left += delta;
  right -= delta;
  
  //Serial.print("[*] Left Speed: ");
  //Serial.print(left);
  //Serial.print(". ");
  //Serial.print("[*] Right Speed: ");
  //Serial.println(right);
  
  if( error < 0 ) Left(delta);
  else if( error > 0 ) Right(delta);
  else Forward(20);
  //if( error != 0 )
  //{
  //  Left(left);
  //  Right(right);
  //}
  //else Forward(100);
  
  delay(500);
}

/**
*
**/
void moveQuadrent2()
{
  
}

/**
*
**/
void moveQuadrent3()
{
  
}

/**
*
**/
void moveQuadrent4()
{
  
}

///////////////////////
// Utility Functions //
///////////////////////

/**
 * Sends the location of the quadrent to the base station
**/
void sendQuadrent()
{
  Serial.print("Current Quadrent: ");
  Serial.print(quadrent);
}

void Forward(int dt)
{
   digitalWrite(dir_a, HIGH);   // select motor direction
   digitalWrite(dir_b, HIGH);   
   analogWrite(pwm_a, move_pulse);     // set motor voltage to max
   analogWrite(pwm_b, move_pulse);
   delay(dt);                   // wait
   //analogWrite(pwm_a, 0);       // set motor voltage to 0
   //analogWrite(pwm_b, 0);
   Serial.println(" forward");
}

void Back(int dt)
{
  digitalWrite(dir_a, LOW);   
  digitalWrite(dir_b, LOW);   
  analogWrite(pwm_a, move_pulse);
  analogWrite(pwm_b, move_pulse);
  delay(dt);
  //analogWrite(pwm_a, 0);
  //analogWrite(pwm_b, 0);
  Serial.println(" back");
}

void Left(int dt)
{
   digitalWrite(dir_a, HIGH);   
   digitalWrite(dir_b, LOW);   
   analogWrite(pwm_a, move_pulse);
   analogWrite(pwm_b, move_pulse);
   delay(dt);
   //analogWrite(pwm_a, 0);
   //analogWrite(pwm_b, 0);
   Serial.println(" left");
}

void Right(int dt)
{
   digitalWrite(dir_a, LOW);   
   digitalWrite(dir_b, HIGH);   
   analogWrite(pwm_a, move_pulse);
   analogWrite(pwm_b, move_pulse);
   delay(dt);
   //analogWrite(pwm_a, 0);
   //analogWrite(pwm_b, 0);
   Serial.println(" right");
}
