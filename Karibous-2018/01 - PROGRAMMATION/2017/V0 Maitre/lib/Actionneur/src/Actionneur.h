#ifndef Actionneur_h
#define Actionneur_h
#include <Servo.h>

class Actionneur
{
public:


	Actionneur(Servo servoActionneur,int pinServo, int vitesse, int min, int max);

	void setAmplitude(int min, int max);
	void setDestination(int destination,int vitesse);
	void setVitesse(int vitesse);

	bool update();

	int readPosition();




protected:


private:
	Servo _servoActionneur;
	int _pinServo;
	int _vitesse;
	int _timePas;
	int _destination;
	int _position;

	int _min;
	int _max;

	int _timeInit ;
	int _timeActu ;

	int _signe ;

};
#endif
