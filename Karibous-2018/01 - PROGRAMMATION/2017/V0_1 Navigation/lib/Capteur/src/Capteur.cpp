#include <Arduino.h>
#include "Capteur.h"

	Capteur::Capteur(unsigned int pin,bool type,bool activatePullup,unsigned int inspectionTime,bool critical )
	{
		_pin = pin ;
		_type = type ;
		_inspectionTime = inspectionTime ;
		_critical = critical ;
		_activatePullup = activatePullup ;

		_readTime = micros();

		_value = 0;

		enableInput();
	}

	void Capteur::setCriticity ( bool critical )
	{
		_critical = critical ;
	}

	bool Capteur::getCriticity()
	{
		return _critical;
	}

	void Capteur::setInspectionTime ( unsigned int inspectionTime )
	{
		_inspectionTime = inspectionTime ;
	}

	unsigned int Capteur::getInspectionTime ()
	{
		return _inspectionTime;
	}

	void Capteur::enableInput()
	{
		if ( _type == DIGITAL )
		{
			if (_activatePullup)
			{
				pinMode(_pin, INPUT_PULLUP);
			}
			else
			{
				pinMode(_pin, INPUT);
			}
		}
	}

	int Capteur::read()
	{
		if (micros()-_readTime>=_inspectionTime)
		{
			if(_type == DIGITAL)
			{
				_value = digitalRead(_pin);
			}
			else
			{
				_value = analogRead(_pin);
			}
			_readTime = micros();
		}
		return _value;
	}
