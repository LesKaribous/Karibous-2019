#include <Arduino.h>
#include <AccelStepper.h>
// Define a stepper and the pins it will use

int pinStep=A2;
int pinDir=A3;
int pinSleep=A1;
int pinReset=A0;

AccelStepper stepper(AccelStepper::DRIVER,pinStep, pinDir); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
void setup()
{
  pinMode(pinReset, OUTPUT);
  pinMode(pinSleep, OUTPUT);
  digitalWrite(pinReset, HIGH);
  digitalWrite(pinSleep, HIGH);
  //
  delay(1000);
  // Change these to suit your stepper if you want
  stepper.setMaxSpeed(700);
  stepper.setAcceleration(10);
  stepper.moveTo(20000);
}
void loop()
{
    // If at the end of travel go to the other end
    if (stepper.distanceToGo() == 0)
      stepper.moveTo(-stepper.currentPosition());
    stepper.run();
}
