#include <QTRSensors.h>

int pwm_a = 3;  //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 11;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int dir_a = 12;  //dir control for motor outputs 1 and 2 is on digital pin 12
int dir_b = 13;  //dir control for motor outputs 3 and 4 is on digital pin 13
int move_pulse = 100;

int lastError = 0;
float KP = 0.2;
float KD = 1;

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
  
  // initializ motor pins  
  pinMode(pwm_a, OUTPUT);  //Set control pins to be outputs
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);
  analogWrite(pwm_a, 0);  // set motor voltages 0
  analogWrite(pwm_b, 0);
  Serial.begin(9600);     // for communacations with computer
  
  //Serial.println("Starting in 3 seconds");
  //delay(3000);
  
}

void loop()
{
  // Get the position of the line
  qtr.read(output);
  
  for( int i = 0; i < numOfSensors; i++ )
  {
   Serial.print(output[i]);
   Serial.print(" ");
   
  }
  
  Serial.println();
  
  int normOutput[numOfSensors];
  
  // Normalise the output into 1 ( white ) and 0 ( black )
  for( int i = 0; i < numOfSensors; i++ )
  {
    if( output[i] > 2000 ) normOutput[i] = 0;
    else normOutput[i] = 1;
  }
  
  for( int i = 0; i < numOfSensors; i++ )
  {
   Serial.print(normOutput[i]);
   Serial.print(" ");
   
  }
  
  Serial.println();
  
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
  int error = 3500 - average;
  
  // Calculate the ammount to change direction by, devide by 100 because a
  // change in the thousands is far to big
  //int delta = abs(error/10);
  int delta = KP * (error/100) + KD * ( (error/100) - lastError );
  lastError = error/100 ;
  
  Serial.print("[*] Error: ");
  Serial.print(error);
  Serial.print(". ");
  Serial.print("[*] Delta: ");
  Serial.println(delta);
  
  // If error is less than 0 then that means that the line towards the right of the sensors
  
  int left = 100;
  int right = 100;
  
  left += delta;
  right -= delta;
  
  //if( error > 0 )
  //{
  //  Serial.println("[*] Turning Right");
    //right += delta;
    //left -= delta;
  //  Right(delta);
  //}
  // This means the line is to the left of the sensors
  //else if( error < 0 )
  //{
  //  Serial.println("[*] Turning Left");
    //right -= delta;
    //left += delta;
  //  Left(delta);
  //}
  
  Serial.print("[*] Left Speed: ");
  Serial.print(left);
  Serial.print(". ");
  Serial.print("[*] Right Speed: ");
  Serial.println(right);
  
  if( error != 0 )
  {
    Left(left);
    Right(right);
  }
  else Forward(100);
  
  delay(400);
}

void Forward(int dt)
{
   digitalWrite(dir_a, LOW);   // select motor direction
   digitalWrite(dir_b, LOW);   
   analogWrite(pwm_a, 255);     // set motor voltage to max
   analogWrite(pwm_b, 255);
   delay(dt);                   // wait
   analogWrite(pwm_a, 0);       // set motor voltage to 0
   analogWrite(pwm_b, 0);
   Serial.println(" forward");
}

void Back(int dt)
{
  digitalWrite(dir_a, HIGH);   
  digitalWrite(dir_b, HIGH);   
  analogWrite(pwm_a, 255);
  analogWrite(pwm_b, 255);
  delay(dt);
  analogWrite(pwm_a, 0);
  analogWrite(pwm_b, 0);
  Serial.println(" back");
}

void Left(int dt)
{
   digitalWrite(dir_a, LOW);   
   digitalWrite(dir_b, HIGH);   
   analogWrite(pwm_a, 255);
   analogWrite(pwm_b, 255);
   delay(dt);
   analogWrite(pwm_a, 0);
   analogWrite(pwm_b, 0);
   Serial.println(" left");
}

void Right(int dt)
{
   digitalWrite(dir_a, HIGH);   
   digitalWrite(dir_b, LOW);   
   analogWrite(pwm_a, 255);
   analogWrite(pwm_b, 255);
   delay(dt);
   analogWrite(pwm_a, 0);
   analogWrite(pwm_b, 0);
   Serial.println(" right");
}
