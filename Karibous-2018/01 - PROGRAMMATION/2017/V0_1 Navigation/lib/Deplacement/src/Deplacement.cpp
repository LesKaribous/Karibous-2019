#include <Arduino.h>
#include <math.h>
#include "Deplacement.h"


Deplacement::Deplacement(int dirRight,
	int dirLeft,
	int stepRight,
	int stepLeft,
	int mode,
	double entraxe,
	double wheelDiameter,
	int reduction)
{
	_pins[0] = dirRight;
	_pins[1] = dirLeft;
	_pins[2] = stepRight;
	_pins[3] = stepLeft;

	_YActu = 0;
	_XActu = 0;

	_entraxe = entraxe;
	_wheelDiameter = wheelDiameter;
	_reduction = reduction;

	_mode = mode;
	setNbStep(NB_STEPPER_TURN);

	_wheelPerimeter = PI * _wheelDiameter ; // En mm
	_stepResolution = _wheelPerimeter/(_nbStep*_reduction); // En mm

	_minPulseWidth = 2; // En us

	enableOutputs();
}

void Deplacement::enableOutputs()
{
	for (int i=0;i<=3;i++)
	{
		pinMode(_pins[i], OUTPUT);
	}
}

void Deplacement::setNbStep(int nbStep)
{
	_nbStep = _mode*nbStep;
}

void Deplacement::setDirection()
{
	// Mise à jour de la direction de rotation
	digitalWrite(_pins[0],_dirMotorRight);
	digitalWrite(_pins[1],!_dirMotorLeft);
	delayMicroseconds(1200);
}

void Deplacement::Step()
{
	// Réalisation d'un pas
	digitalWrite(_pins[2],HIGH);
	digitalWrite(_pins[3],HIGH);
	delayMicroseconds(_minPulseWidth);
	digitalWrite(_pins[2],LOW);
	digitalWrite(_pins[3],LOW);
}

bool Deplacement::run()
{
	if (_currentStep<_targetStep)
	{
		if(_currentStep<_accelDistance)
		{
			// Phase d'acceleration
			_m=-_R;
		}
		else if(_currentStep>=_accelDistance && _currentStep<=_targetStep-_accelDistance)
		{
			// Phase vitesse constante
			_m=0;
		}
		else
		{
			// Phase de decceleration
			_m=_R;
		}
		if ((micros()-_lastTime) >=_Pa)
		{
			Step();
			_lastTime = micros();
			_currentStep=_currentStep+1;
			// Equation 22 :
			_q = _m*_Pa*_Pa;
			// Equation 23 :
			_Pa = _Pa*(1+_q+_q*_q);
		}
		_stateRun = true;
	}
	else if(_statePause)
	{
		_Pa = _P1; // Pret à repartir
	}
	else
	{
		_stateRun = false;
		_Pa = _P1; // Pret à repartir
	}
	return _stateRun;
}

void Deplacement::setMaxSpeed(unsigned long speed)
{
	_speed = speed;
	computeSpeedAccel();
}

void Deplacement::setAcceleration(unsigned long accel)
{
	_accel = accel;
	computeSpeedAccel();
}

void Deplacement::turn(long angle)
{
	_targetStep=abs(angle);
	_currentStep=0;

	if (angle > 0)
	{
		_dirMotorLeft = 1;
		_dirMotorRight = 0;
	}
	else
	{
		_dirMotorLeft = 0;
		_dirMotorRight = 1;
	}
	setDirection();
}

void Deplacement::go(long distance)
{
	_targetStep = abs(distance);
	_currentStep = 0;

	if (distance > 0)
	{
		_dirMotorLeft = 1;
		_dirMotorRight = 1;
	}
	else
	{
		_dirMotorLeft = 0;
		_dirMotorRight = 0;
	}
	setDirection();
}

void Deplacement::computeSpeedAccel()
{
	// Equation 17
	_P1 = 1000000/sqrt(2*_accel);
	_Pa=_P1; // Deplacement initial
	// Equation 19
	_R=_accel/pow(1000000,2);
	//Equation 16
	_accelDistance=(pow(_speed,2)/(2*_accel));
}

void Deplacement::pause()
{
	if (!_statePause)
	{
		_targetStepTemp = _targetStep ;
		_currentStepTemp = _currentStep + _accelDistance ;
		_targetStep = _currentStep + _accelDistance ;
		_statePause = true ;
		Serial.print("Pause :");
		Serial.println(_targetStep);
	}
}

void Deplacement::resume()
{
	if (_statePause)
	{
		_targetStep = _targetStepTemp - _currentStepTemp ;
		_currentStep = 0 ;
		_statePause = false ;
		_stateGoTo=1;
		Serial.print("Resume :");
		Serial.println(_targetStep);
	}
}

bool Deplacement::readPause()
{
	return _statePause;
}

void Deplacement::turnGo(long angle, long distance)
{
	_targetDistStep = double(distance/_stepResolution);

	// Calcul de la longueur d'arc à effectuer pour réaliser la rotation
	_arcDistance = ( ( angle / 180.0 ) * PI ) * (_entraxe / 2.0) ; // En mm
	_targetArcStep = double(_arcDistance/_stepResolution) ; // Conversion en pas

	_stateGoTo=1;
}

bool Deplacement::runGoTo()
{
	switch (_stateGoTo)
	{
		case 0:
			//Stop
		break;
		case 1:
			//Init turn
			turn(_targetArcStep);
			_stateGoTo++;
		case 2:
			//En train de tourner
			if(!run()) _stateGoTo++; // Le robot est arrivé
		break;
		case 3:
			// Init go distance
			go(_targetDistStep);
			_stateGoTo++;
		break;
		case 4:
			//En train d'avancer
			if(!run()) _stateGoTo = 0; // Le robot est arrivé
		break;
	}

	if(_stateGoTo == 0 && !_statePause)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Deplacement::goTo(long X, long Y, long orientation)
{
	_distance = sqrt(pow((Y-_YActu),2)+pow((X-_XActu),2)); // En mm
	_angle = restPmPi(atan2(Y-_YActu,X-_XActu)-_angle); // En radians

	_XActu = X;
	_YActu = Y;

	_targetDistStep = double(_distance/_stepResolution);

	// Calcul de la longueur d'arc à effectuer pour réaliser la rotation
	_arcDistance = _angle * (_entraxe / 2.0) ; // En mm
	_targetArcStep = double(_arcDistance/_stepResolution) ; // Conversion en pas

	Serial.println(_angle);

	_stateGoTo=1;
}

double Deplacement::restPmPi(double angle)
{
	double newAngle = angle;
	if ( newAngle > PI)
	{
		while ( newAngle > PI) newAngle = PI - angle - PI;
	}
	else if ( newAngle < -PI )
	{
		while ( newAngle < -PI) newAngle = PI + angle + PI;
	}
	return newAngle;
}
