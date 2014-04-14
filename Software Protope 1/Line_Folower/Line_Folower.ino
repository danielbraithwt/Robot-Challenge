#include <QTRSensors.h>

int pwm_a = 3;  //PWM control for motor outputs 1 and 2 is on digital pin 3
int pwm_b = 11;  //PWM control for motor outputs 3 and 4 is on digital pin 11
int dir_a = 12;  //dir control for motor outputs 1 and 2 is on digital pin 12
int dir_b = 13;  //dir control for motor outputs 3 and 4 is on digital pin 13
int move_pulse = 100;

// Number of sensors
int numOfSensors = 9;
// Sensor Pins ( Digital Pins )
int pins[] = { 13, 12, 11, 10, 9, 8, 7, 6, 5 };
// Create array for output from sensors
unsigned int output[9];

// Create the sensor object
QTRSensorsRC qtr((unsigned char[]) { 13, 12, 11, 10, 9, 8, 7, 6, 5 }, numOfSensors);

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
  // Get the position of the line
  int pos = qtr.readLine(output, 1);
  
  // Calculate the error
  int error = pos - 5000;
  
  Serial.print("Error: ");
  Serial.println(error);
  
  if( error < 0 )
  {
    Left(error/1000);
  }
  else if( error > 0 )
  {
    Right(error/1000);
  }
  Forward(100);
  
  delay(50);
}

void Forward(int dt)
{
   digitalWrite(dir_a, HIGH);   // select motor direction
   digitalWrite(dir_b, HIGH);   
   analogWrite(pwm_a, 255);     // set motor voltage to max
   analogWrite(pwm_b, 255);
   delay(dt);                   // wait
   analogWrite(pwm_a, 0);       // set motor voltage to 0
   analogWrite(pwm_b, 0);
   Serial.println(" forward");
}

void Back(int dt)
{
  digitalWrite(dir_a, LOW);   
  digitalWrite(dir_b, LOW);   
  analogWrite(pwm_a, 255);
  analogWrite(pwm_b, 255);
  delay(dt);
  analogWrite(pwm_a, 0);
  analogWrite(pwm_b, 0);
  Serial.println(" back");
}

void Left(int dt)
{
   digitalWrite(dir_a, HIGH);   
   digitalWrite(dir_b, LOW);   
   analogWrite(pwm_a, 255);
   analogWrite(pwm_b, 255);
   delay(dt);
   analogWrite(pwm_a, 0);
   analogWrite(pwm_b, 0);
   Serial.println(" left");
}

void Right(int dt)
{
   digitalWrite(dir_a, LOW);   
   digitalWrite(dir_b, HIGH);   
   analogWrite(pwm_a, 255);
   analogWrite(pwm_b, 255);
   delay(dt);
   analogWrite(pwm_a, 0);
   analogWrite(pwm_b, 0);
   Serial.println(" right");
}
