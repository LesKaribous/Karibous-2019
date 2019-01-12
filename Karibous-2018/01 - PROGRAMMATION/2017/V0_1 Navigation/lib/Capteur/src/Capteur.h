#ifndef Capteur_h
#define Capteur_h

#define ANALOG 0
#define DIGITAL 1

class Capteur
{
public:


	Capteur(unsigned int pin,bool type,bool activatePullup,unsigned int inspectionTime,bool critical);

	void enableInput();

	int read();

	void setCriticity ( bool critical );

	bool getCriticity ();

	void setInspectionTime ( unsigned int inspectionTime );

	unsigned int getInspectionTime ();


protected:



private:

	unsigned int _pin;
	bool _type;
	unsigned int _inspectionTime;
	bool _critical;
	bool _activatePullup;
	unsigned long _readTime;
	int _value;


};
#endif
