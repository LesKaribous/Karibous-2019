#include <Arduino.h>
#include <AccelStepper.h>

int pinStep1=4;
int pinDir1=5;
int pinSleep1=3;
int pinReset1=2;

int pinStep2=8;
int pinDir2=9;
int pinSleep2=7;
int pinReset2=6;

AccelStepper stepper1(AccelStepper::DRIVER,pinStep1, pinDir1);
AccelStepper stepper2(AccelStepper::DRIVER,pinStep2, pinDir2);

void setup()
{
    pinMode(pinReset1, OUTPUT);
    pinMode(pinSleep1, OUTPUT);
    pinMode(pinReset2, OUTPUT);
    pinMode(pinSleep2, OUTPUT);

    digitalWrite(pinReset1, HIGH);
    digitalWrite(pinSleep1, HIGH);
    digitalWrite(pinReset2, HIGH);
    digitalWrite(pinSleep2, HIGH);

    stepper1.setMaxSpeed(300.0);
    stepper1.setAcceleration(200.0);
    stepper1.moveTo(-1000000);

    stepper2.setMaxSpeed(200.0);
    stepper2.setAcceleration(100.0);
    stepper2.moveTo(1000000);
}
void loop()
{
    // Change direction at the limits
    if (stepper1.distanceToGo() == 0)
        stepper1.moveTo(-stepper1.currentPosition());
    stepper1.run();
     stepper2.run();
}
