#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60
#define FINI 0
#define EN_COURS 1
#define PREVU 2

int pinStep1=4;
int pinDir1=5;
int pinSleep1=3;
int pinReset1=2;

int pinStep2=8;
int pinDir2=9;
int pinSleep2=7;
int pinReset2=6;

AccelStepper MGauche(AccelStepper::DRIVER,pinStep1, pinDir1);
AccelStepper MDroit(AccelStepper::DRIVER,pinStep2, pinDir2);

byte fonction ;
int16_t absoluteRequest[3] ; // X,Y,orientation
int16_t relativeRequest[2] ; // rotation, distance

bool newPos = false;
byte etat = 0;
bool PRESENCE_ARRIERE = 0, PRESENCE_AVANT = 0;
int ADVERSAIRE_ARRIERE = 22;
int ADVERSAIRE_AVANT = 23;

double AskX, AskRot, TempGauche, TempDroit, NewX, NewRot ;

int sensorTime = 2000;
int avantTimeInit = 0;
int arriereTimeInit = 0;

bool optionAdversaire = false;
bool optionRecalage = false;
bool optionRalentit = false;

char etatRotation, etatAvance;

const float FacteurX = 77.44; //Ancien : 154.8
const float FacteurDroit = 8.0; //Ancien : 154.8
const float FacteurGauche = 8.0; //Ancien : 154.8
const float FacteurRot = 13.98; //Ancien : 19.64

const float VitesseMaxDroite = 3500.0; //Ancien : 8000
const float VitesseMaxGauche = 3500.0; //Ancien : 8000
const float VitesseMinDroite = 500.0; //Ancien : 5000
const float VitesseMinGauche = 500.0; //Ancien : 5000
const float AccelRot = 1800.0; //Ancien : 2000
const float AccelMin = 400.0; //Ancien : 2000
const float AccelMax = 1500.0; //Ancien : 5000
const float AccelStop = 4000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
//int PIN_BORDURE[4] = {20,17,16,21};

void receiveEvent(int HowMany);
void requestEvent();
void adversaire();
void updatePos();
void bordure();
void turnGo();
void recalage();
void FIN_MATCH();

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

	Serial.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	MGauche.setMaxSpeed(VitesseMaxGauche);
	MGauche.setAcceleration(AccelMax);

	MDroit.setMaxSpeed(VitesseMaxDroite);
	MDroit.setAcceleration(AccelMax);

	// pinMode(ADVERSAIRE_ARRIERE, INPUT_PULLUP);
	// pinMode(ADVERSAIRE_AVANT, INPUT_PULLUP);
	//
	// for(int i = 0;i<4;i++)
	// {
	// 	pinMode(PIN_BORDURE[i], INPUT_PULLUP);
	// }
}

void loop()
{
	MGauche.run();
	MDroit.run();

	updatePos();
	adversaire();
	turnGo();
	bordure();

	if (fonction == 255)
	{
		FIN_MATCH();
		MGauche.stop();
		MDroit.stop();
		MGauche.run();
		MDroit.run();
	}

}

void updatePos()
{
	if (newPos)
	{
		etatRotation = PREVU ;
		etatAvance = PREVU ;
		// Traitement de l'information
		NewX=relativeRequest[1];
		NewRot=relativeRequest[0];
	}
	newPos = false;
}

void turnGo()
{
  if ((PRESENCE_AVANT && NewX>=0 && etatAvance == EN_COURS) || (PRESENCE_ARRIERE && NewX<0 && etatAvance == EN_COURS) )
  {
     TempGauche = MGauche.distanceToGo();
     TempDroit = MDroit.distanceToGo();

     MGauche.setAcceleration(AccelStop);
     MDroit.setAcceleration(AccelStop);

     MGauche.move(0); // Stop as fast as possible: sets new target
     MDroit.move(0); // Stop as fast as possible: sets new target

     TempGauche = TempGauche + MGauche.distanceToGo();
     TempDroit = TempDroit + MDroit.distanceToGo();

     MGauche.run();
     MDroit.run();

     // Attendre que l'adversaire soit partit
     for(unsigned long i=0;i<=80000;i++) //Attendre XXXX iterations avant de recommencer
     {
       adversaire();
       if ((PRESENCE_AVANT &&  NewX>=0 ) || (PRESENCE_ARRIERE && NewX<0))
       {
          i=0;              //RAZ de l'iteration si toujours un obstacle
       }
       //delayMicroseconds(10);
       MGauche.run();
       MDroit.run();
     }

     MGauche.setAcceleration(AccelMax);
     MDroit.setAcceleration(AccelMax);

     MGauche.move(TempGauche);
     MDroit.move(TempDroit);
  }
  else
  {
    if (etatRotation == PREVU)
    {
      MGauche.setAcceleration(AccelRot);
      MDroit.setAcceleration(AccelRot);
      MDroit.move(NewRot*FacteurRot);
      MGauche.move(NewRot*FacteurRot);
      etatRotation = EN_COURS ;
    }
    if (MDroit.distanceToGo() == 0 && MGauche.distanceToGo() == 0 && etatRotation == EN_COURS)
    {
      etatRotation = FINI ;
      etatAvance = PREVU ;
    }
    if (etatAvance == PREVU && etatRotation == FINI)
    {
		if (optionRalentit)
		{
			MGauche.setMaxSpeed(VitesseMinGauche);
	        MDroit.setMaxSpeed(VitesseMinDroite);
	        MGauche.setAcceleration(AccelMin);
	        MDroit.setAcceleration(AccelMin);
		}
		else
		{
			MGauche.setMaxSpeed(VitesseMaxGauche);
	        MDroit.setMaxSpeed(VitesseMaxDroite);
	        MGauche.setAcceleration(AccelMax);
	        MDroit.setAcceleration(AccelMax);
		}

      MDroit.move(NewX*FacteurDroit);
      MGauche.move(-(NewX*FacteurGauche));
      etatAvance = EN_COURS ;
    }
    if (etatAvance == EN_COURS && optionRecalage)
    {
      MGauche.setMaxSpeed(VitesseMinGauche);
      //MGauche.setAcceleration(AccelMax);
      MDroit.setMaxSpeed(VitesseMinDroite);
      //MDroit.setAcceleration(AccelMax);
	  MGauche.setAcceleration(AccelMin);
	  MDroit.setAcceleration(AccelMin);
      // Si on est à la fin du mouvement
      recalage();
    }
    if (MDroit.distanceToGo() == 0 && MGauche.distanceToGo() == 0 && etatAvance == EN_COURS)
    {
      etatAvance = FINI ;
    }
  }
}

void recalage()
{
	// A MODIFIER !!!!!!!!
	// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
   if ( (bitRead(BORDURE,0) && NewX > 0 ) || ( bitRead(BORDURE,3) && NewX < 0))
  {
    MDroit.setAcceleration(AccelStop);
    MDroit.move(0); // Stop as fast as possible: sets new target
    MDroit.setCurrentPosition(0);
    while (MDroit.distanceToGo() != 0 )
    {
      MDroit.run();
    }
    MDroit.setMaxSpeed(VitesseMaxDroite);
    MDroit.setAcceleration(AccelMax);
  }
   if ( (bitRead(BORDURE,1) && NewX > 0) || (bitRead(BORDURE,2) && NewX < 0))
  {
    MGauche.setAcceleration(AccelStop);
    MGauche.move(0); // Stop as fast as possible: sets new target
    MGauche.setCurrentPosition(0);
    while (MGauche.distanceToGo() != 0 )
    {
      MGauche.run();
    }
    MGauche.setMaxSpeed(VitesseMaxGauche);
    MGauche.setAcceleration(AccelMax);
  }
}

void bordure()
{
	// for(int i = 0;i<4;i++)
	// {
	// 	bitWrite(BORDURE,i,digitalRead(PIN_BORDURE[i]));
	// }
	// //Serial.println(BORDURE,BIN);
	// //delay(200);

}

void adversaire()
{
	// // Si la detection adverse est activée
	// if (!optionAdversaire)
	// {
	// 	// Adversaire Avant
	// 	if (digitalReadFast(ADVERSAIRE_AVANT))
	// 	{
	// 		if (!PRESENCE_AVANT)
	// 		{
	// 			PRESENCE_AVANT = true ;
	// 		}
	// 		avantTimeInit = millis();
	// 	}
	// 	else
	// 	{
	// 		if((millis()-avantTimeInit)>=sensorTime)
	// 		{
	// 			PRESENCE_AVANT = false;
	// 		}
	// 	}
	// 	// Adversaire Arriere
	// 	if (digitalReadFast(ADVERSAIRE_ARRIERE))
	// 	{
	// 		if (!PRESENCE_ARRIERE)
	// 		{
	// 			PRESENCE_ARRIERE = true ;
	// 		}
	// 		arriereTimeInit = millis();
	// 	}
	// 	else
	// 	{
	// 		if((millis()-arriereTimeInit)>=sensorTime)
	// 		{
	// 			PRESENCE_ARRIERE = false;
	// 		}
	// 	}
	// }
	// else
	// {
	// 	PRESENCE_ARRIERE = false;
	// 	PRESENCE_AVANT = false;
	// }
	//
	// // UNIQUEMENT EN DEBUG !!!!!!!!!!!
	// /*
	// if (PRESENCE_AVANT || PRESENCE_ARRIERE)
	// {
	// 	Serial.print(PRESENCE_AVANT);
	// 	Serial.print(" - ");
	// 	Serial.println(PRESENCE_ARRIERE);
	// 	delay(200);
	// }
	// */

}

void receiveEvent(int howMany)
{
	if(howMany == 7)
	{
		// Si un déplacement absolue est demandé
		fonction = Wire.read();
		absoluteRequest[0]= Wire.read() << 8 | Wire.read();
		absoluteRequest[1]= Wire.read() << 8 | Wire.read();
		absoluteRequest[2]= Wire.read() << 8 | Wire.read();
		/*
		Serial.print(absoluteRequest[0]);
		Serial.print(" / ");
		Serial.print(absoluteRequest[1]);
		Serial.print(" / ");
		Serial.println(absoluteRequest[2]);
		*/
		etat=1;
	}
	else if(howMany == 5)
	{
		// Si un déplacement relatif est demandé
		fonction = Wire.read();
		relativeRequest[0]= Wire.read() << 8 | Wire.read();
		relativeRequest[1]= Wire.read() << 8 | Wire.read();
		/*
		Serial.print(relativeRequest[0]);
		Serial.print(" / ");
		Serial.println(relativeRequest[1]);
		*/
		etat=1;
	}
	newPos = true;
	optionAdversaire = bitRead(fonction, 0);
	optionRecalage = bitRead(fonction, 1);
	optionRalentit = bitRead(fonction,2);
	if (fonction==255)
	{
		// Stop le robot le plus rapidement possible
	}
}

//Fin de match
void FIN_MATCH()
{
	MGauche.setSpeed(VitesseMaxGauche);
	MDroit.setSpeed(VitesseMaxDroite);
	MGauche.setAcceleration(AccelStop);
	MDroit.setAcceleration(AccelStop);
 	MGauche.move(0);    //Commande de deplacement Relatif
   	MDroit.move(0);      //Commande de deplacement Relatif

   	while(1)
   	{
		MGauche.stop();
		MDroit.stop();
    	MGauche.run();
    	MDroit.run();
   	}
}

void requestEvent()
{
	if ( etatAvance == FINI && etatRotation == FINI)
  	{
		Wire.write(0);
	}
	else
	{
		Wire.write(1);
	}
}
