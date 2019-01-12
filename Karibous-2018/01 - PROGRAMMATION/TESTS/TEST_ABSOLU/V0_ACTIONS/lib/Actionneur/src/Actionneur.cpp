#include <Arduino.h>
#include "Actionneur.h"
#include <Servo.h>


Actionneur::Actionneur(Servo servoActionneur,int pinServo, int vitesse, int min, int max)
{
	_servoActionneur = servoActionneur ;
	setVitesse(vitesse);
	_pinServo = pinServo ;
	_min = min;
	_max = max;

	if (!_servoActionneur.attached())
	{
		_servoActionneur.attach(_pinServo,_min,_max);
	}
}

void Actionneur::setAmplitude(int min, int max)
{
	_min = min;
	_max = max;
}
void Actionneur::setDestination(int destination,int vitesse)
{
	_destination = destination;
	_position = _servoActionneur.readMicroseconds();
	if ( (_destination - _position) < 0 )
	{
		_signe = -1 ;
	}
	else
	{
		_signe = 1 ;
	}
	setVitesse(vitesse);
}
void Actionneur::setVitesse(int vitesse)
{
	_vitesse = vitesse;
	if (_vitesse >= 300) _vitesse = 300 ;
	_timePas = 1000000 / ( 5 * _vitesse );
}

int Actionneur::readPosition()
{
	return _position;
}

bool Actionneur::update()
{
	if (_position != _destination)
	{
		if ((micros()-_timeActu)>=_timePas)
		{
			_position = _position +( 1 * _signe);
			_servoActionneur.writeMicroseconds(_position);
			_timeActu = micros();
		}
		return true;
	}
	else
	{
		return false;
	}

}
