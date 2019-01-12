#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
#include <Actionneur.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 80
//Etat des actions
#define FINI 0
#define EN_COURS 1
#define PREVU 2
//Etat de la nouvelle action demandée
#define VALIDEE 0 // Nouvelle action validée et prise en compte
#define DISPONIBLE 1 // Nouvelle action enregistrée
#define ERRONEE 2 // nouvelle action erronée. CRC nok.

// Declaration des borches d'IO
//int digi_1 = 7, digi_2 = 6, digi_3 = 5, digi_4 = 4, digi_5 = 3, digi_6 = 2, digi_7 = 9, digi_8 = 8;
//int ana_1 = A8, ana_2 = A9, ana_3 = A0, ana_4 = A1, ana_5 = A2, ana_6 = A3, ana_7 = A6, ana_8 = A7; //A refaire

// Pin IO pour le moteur pas-à-pas du barillet
int pinStep1=4, pinDir1=5, pinSleep1=3, pinReset1=2;
// Declaration du moteur barillet
AccelStepper MBarillet(AccelStepper::DRIVER,pinStep1, pinDir1);

FastCRC8 CRC8;
byte bufAction[2]={0,0}; // Buffer de reception des ordres d'action + le CRC
byte crcAction = 0; // CRC de controle des ordres d'action'
byte etatAction ;

const float VitesseMaxBarillet = 1000.0; //Ancien :
const float VitesseMinBarillet = 500.0; //Ancien :
const float AccelMin = 300.0; //Ancien :
const float AccelMax = 800.0; //Ancien :
const float AccelStop = 2000.0; //Ancien :

int16_t actionRequest ; // action

byte newAction = VALIDEE;

void receiveEvent(int HowMany);
void requestEvent();
void updateAction();
void selectAction();
void finMatch();

void setup()
{
  pinMode(pinReset1, OUTPUT);
	pinMode(pinSleep1, OUTPUT);

	digitalWrite(pinReset1, HIGH);
	digitalWrite(pinSleep1, HIGH);

	Serial.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

  MBarillet.setMaxSpeed(VitesseMaxBarillet);
  MBarillet.setAcceleration(AccelMax);
}

void loop()
{
  updateAction();
	if (actionRequest == 255) finMatch();
}

void updateAction()
{
	if (newAction==DISPONIBLE)
	{
		etatAction = PREVU ;
	}
	newAction = VALIDEE;
}

void receiveEvent(int howMany)
{
	if(howMany == 2)
	{
		// Si un déplacement relatif est demandé
		// On receptionne les données
		for (int i=0;i<=1;i++)
		{
			bufAction[i]=Wire.read();
		}
	}
	// On calcul le CRC
	crcAction = CRC8.smbus(bufAction, sizeof(bufAction)-1); //On enleve le CRC
	//Serial.println(crcAction);
	// On regarde si le CRC calculé correspond à celui envoyé
	if (crcAction==bufAction[1])
	{
		// CRC ok
		// On traite les données
		actionRequest = bufAction[0];
		// On indique qu'une nouvelle action est disponible
		newAction = DISPONIBLE;
	}
	else
	{
		// CRC nok - la donnée est erronée
		// On indique que la prochaine action est erronée pour en renvoi eventuel
		newAction = ERRONEE;
	}

}

//Fin de match
void finMatch()
{
   	while(1)
   	{
		    // Couper tous les servomoteurs
   	}
}

void requestEvent()
{

	if ( etatAction == FINI && newAction == VALIDEE)
  {
    // Action terminée
		Wire.write("O");
		//Serial.println('O');
	}
	else if (newAction == ERRONEE)
	{
    // Commande non validé
		Wire.write("E");
		//Serial.println('N');
	}
	else
	{
    // Action non terminée
		Wire.write("N");
		//Serial.println('N');
	}
}
