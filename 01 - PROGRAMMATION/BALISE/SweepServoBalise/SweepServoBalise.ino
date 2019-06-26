#include <VarSpeedServo.h> 
 
VarSpeedServo myservo;
 
const int servoPin = 9; 
const int led = 13;
const int control = A2;

const int position1 = 160;
const int position2 = 50;
 
void setup() 
{ 
  myservo.attach(servoPin);
  delay(1000);
  
  pinMode(led,OUTPUT);
  pinMode(control,INPUT_PULLUP);
  
  for(int i=0; i<=10; i++)
  {
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
  }
} 

void loop() 
{
  if (digitalRead(control))
  {
    digitalWrite(led,HIGH);
    myservo.write(position1,100,true);
    //delay(400);
    myservo.write(position2,100,true);
    //delay(400);
  }
  else
  {
    digitalWrite(led,LOW);
  }
} 
