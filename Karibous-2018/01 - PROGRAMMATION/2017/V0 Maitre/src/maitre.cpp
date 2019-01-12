#include <Arduino.h>
#include <Capteur.h>
#include <Wire.h>
#include <Servo.h>
#include <Actionneur.h>
#include "stdlib.h"

// Adressage I2C pour les cartes esclaves
#define NAVIGATION 60
#define IHM 80
// Boutons
#define TEAM 0
#define STRATEGIE_1 1
#define STRATEGIE_2 2
#define ADVERSAIRE 3
// Couleur Equipe
#define BLEU 1
#define JAUNE 0
//
#define INSERE_TIRETTE 0
#define ENLEVE_TIRETTE 1
#define DEBUT_MATCH 2

#define RECALAGE_BORDURE 1

const int pinPinceGauche = 23;
const int pinPinceDroite = 22;
const int pinRouleauBalle = 11;
const int pinBascule = 12;
const int pinPont = 5;
const int pinArceau = 6;
const int pinEjecteurBalle = 20;
const int pinSouffleur = 21;
const int pinBalise = 15;

const int pinCapteurModule = 14;
const int pinTirette = 10;

const int pinceGaucheOuverte = 2150 ;
const int pinceDroiteOuverte = 1250 ;

const int pinceGaucheFerme = 2050 ;
const int pinceDroiteFerme = 1350 ;

const int pinceGaucheRangee = 1050 ;
const int pinceDroiteRangee = 2100 ;

const int rouleauAval = 1200 ;
const int rouleauRecrache = 2000 ;
const int rouleauStop = 1706 ;

const int basculeBas = 1230;
const int basculeHaut = 2000;

const int pontBas = 2350;
const int pontHaut = 1650;

const int arceauBas = 1200;
const int arceauHaut = 2150;

const int baliseMilieu = 1400;
const int baliseAmplitude = 200;

const int ejecteurON = 127;
const int ejecteurOFF = 0;

const int souffleurON = 255;
const int souffleurOFF = 0;

bool equipe = BLEU ;
bool button[4];
byte optionNavigation = 0;

bool baliseState = 0;
unsigned int baliseInitTime = 0;
const int baliseTime = 1000 ;

bool etatNavigation = 0;

const double TEMPS_MATCH = 89000;
double timeInit=0;

void sendNavigation(byte fonction, int X, int Y, int rot);
void sendNavigation(byte fonction, int rot, int dist);
void sendIHM(byte fonction);
void askIHM();
bool mouvement(Servo myServo, int destination, int vitesse);
bool askNavigation();
void updateBalise();

void initActionneur();
void ASPIRATION_BALLE(int k);
void EJECTION_BALLE(int temps);
void attente(int temps);
void testDeplacement();
void TURN_GO(bool recalage,bool ralentit,int turn, int go);
void OUVRIR_PINCE();
void FERMER_PINCE();
void RANGER_PINCE();
void STRATEGIE_HOMOLOGATION();
void STRATEGIE_HOMOLOGATION_2();
int UPDATE_TEMPS();
void REJET_BALLE(int temps);
void REJET_BALLE_DEBUT();
void REJET_BALLE_FIN();

void recalageInit();

void FIN_MATCH();

Servo pinceGauche;
Servo pinceDroite;
Servo rouleauBalle;
Servo pont;
Servo bascule;
Servo arceau;
Servo balise;

Actionneur baliseA(balise,pinBalise,100,1000,2000);

void setup()
{
	Wire.begin();
	Serial.begin(9600);

	pinMode(pinTirette, INPUT);
	pinMode(pinCapteurModule, INPUT_PULLUP);

	pinMode(pinSouffleur, OUTPUT);
	pinMode(pinEjecteurBalle, OUTPUT);

	initActionneur();

	//////// A ENLEVER /////////
	/*
	arceau.attach(pinArceau,1000,2500);
	arceau.writeMicroseconds(arceauBas+200);
	analogWrite(pinSouffleur,255);
	attente(2000);
	while(1)
	{
		rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
		rouleauBalle.writeMicroseconds(rouleauAval);
		delay(10000);
	}
	*/
	//////// A ENLEVER /////////

	recalageInit();

	sendIHM(INSERE_TIRETTE);
	while(!digitalRead(pinTirette))
	{
		//Attente insertion tirette
	}
	sendIHM(ENLEVE_TIRETTE);
	while(digitalRead(pinTirette))
	{
		//attente relache tirette
		askIHM();
		delay(100);
		//attente(100);
	}
	timeInit = millis();
	askIHM();
	sendIHM(DEBUT_MATCH);
}

void loop()
{

	//ASPIRATION_BALLE(10000);
	//EJECTION_BALLE(3000);
	//testDeplacement();
	STRATEGIE_HOMOLOGATION_2();
	//OUVRIR_PINCE();
	//FERMER_PINCE();
	//attente(1000);

}

void recalageInit()
{
	// Recalage arriere
	//TURN_GO(true,true,0,-200);
	//TURN_GO(false,true,0,30);
	//TURN_GO(true,true,0,-60);
	TURN_GO(false,true,0,-100);

	attente(1000);
	TURN_GO(false,true,0,50);
	/*
	// Recalage droite
	TURN_GO(false,true,-90,0);
	TURN_GO(true,true,0,-200);
	TURN_GO(false,true,0,30);
	TURN_GO(true,true,0,-60);
	TURN_GO(false,true,0,-20);

	attente(1000);
	TURN_GO(false,true,0,50);
	// Recalage arriere
	TURN_GO(false,true,90,0);
	TURN_GO(true,true,0,-200);
	TURN_GO(false,true,0,30);
	TURN_GO(true,true,0,-60);
	TURN_GO(false,true,0,-20);

	attente(1000);
	TURN_GO(false,true,0,20);
	*/
}

void STRATEGIE_HOMOLOGATION()
{
	// Sortie de zone de départ
	TURN_GO(0,false,0,330);
	// Placement sur le premier cratère et aspiration
	TURN_GO(0,false,90,-100);
	ASPIRATION_BALLE(3);
	TURN_GO(0,false,0,100);
	TURN_GO(0,false,-90,300);
	TURN_GO(0,false,-90,630);
	TURN_GO(0,false,-90,300);
	// Dépose 1er cratère
	EJECTION_BALLE(3000);
	// Aller-retour grand cratère
	for (int i = 0;i<=3;i++)
	{
		TURN_GO(0,false,0,-400);
		TURN_GO(0,false,-90,0);
		REJET_BALLE(2000);
		TURN_GO(0,false,90,-500);
		TURN_GO(0,false,-30,-80);
		ASPIRATION_BALLE(3);
		TURN_GO(0,false,0,80);
		TURN_GO(0,false,30,900);
		EJECTION_BALLE(3000);
	}

	FIN_MATCH();
}

void STRATEGIE_HOMOLOGATION_2()
{
	// Sortie de zone de départ
	TURN_GO(0,false,0,330);
	// Placement sur le premier cratère et aspiration
	TURN_GO(0,false,90,-100);
	ASPIRATION_BALLE(3);
	TURN_GO(0,false,0,100);
	TURN_GO(0,false,-90,300);
	TURN_GO(0,false,-90,400);
	TURN_GO(0,false,-45,300);
	// Pousse le cylindre
	TURN_GO(0,false,45,60);
	TURN_GO(0,false,0,-50);
	TURN_GO(0,false,-90,120);
	TURN_GO(0,false,0,-25); // RECALAGE BORDURE
	//TURN_GO(0,false,-90,630);
	//TURN_GO(0,false,-90,300);
	// Dépose 1er cratère
	EJECTION_BALLE(3000);
	// Aller-retour grand cratère
	for (int i = 0;i<1;i++)
	{
		TURN_GO(0,false,0,15);
		REJET_BALLE_DEBUT();
		TURN_GO(0,false,0,-890);
		REJET_BALLE_FIN();
		//TURN_GO(0,false,0,-400);
		//TURN_GO(0,false,90,0);
		//REJET_BALLE(2000);
		//TURN_GO(0,false,-90,-500);
		//TURN_GO(0,false,0,-500);
		TURN_GO(0,false,-22,-60);
		ASPIRATION_BALLE(3);
		TURN_GO(0,false,0,60);
		TURN_GO(0,false,22,890);
		TURN_GO(0,false,0,-25);  // RECALAGE BORDURE
		EJECTION_BALLE(3000);
	}
	// On va pousser le cylindre

	TURN_GO(0,false,0,15);
	TURN_GO(0,false,0,-805);
	TURN_GO(0,false,-90,400);
	TURN_GO(0,true,0,20);
	OUVRIR_PINCE();
	TURN_GO(0,true,-40,0);
	TURN_GO(0,true,40,0);
	FERMER_PINCE();
	TURN_GO(0,true,0,-200);

	FIN_MATCH();
}

void OUVRIR_PINCE()
{
	pinceDroite.attach(pinPinceDroite, 1000, 2200);
	pinceDroite.writeMicroseconds(pinceDroiteOuverte);
	delay(1000);
	pinceGauche.attach(pinPinceGauche, 1000, 2200);
	pinceGauche.writeMicroseconds(pinceGaucheOuverte);
	delay(1000);
}

void FERMER_PINCE()
{
	while(1)
	{
		while(!digitalRead(pinCapteurModule))
		{
			pinceGauche.writeMicroseconds(pinceGaucheOuverte);
			pinceDroite.writeMicroseconds(pinceDroiteOuverte);
			delay(500);
		}
		while(digitalRead(pinCapteurModule))
		{
			pinceGauche.writeMicroseconds(pinceGaucheFerme);
			pinceDroite.writeMicroseconds(pinceDroiteFerme);
			delay(500);
		}
	}
}

void RANGER_PINCE()
{
	pinceGauche.attach(pinPinceGauche, 1000, 2200);
	pinceGauche.writeMicroseconds(pinceGaucheRangee);
	delay(1000);
	pinceDroite.attach(pinPinceDroite, 1000, 2200);
	pinceDroite.writeMicroseconds(pinceDroiteRangee);
	delay(1000);
	pinceDroite.detach();
	pinceGauche.detach();
}

void testDeplacement()
{
	sendNavigation(1, 0, 300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
	sendNavigation(1, 0,-300);
	attente(2000);
	while(askNavigation())
	{
		attente(100);
		Serial.println(askNavigation());
	}
}

void TURN_GO(bool recalage,bool ralentit,int turn, int go)
{
	bitWrite(optionNavigation,0,button[ADVERSAIRE]);
	bitWrite(optionNavigation,1,recalage);
	bitWrite(optionNavigation,2,ralentit);
	sendNavigation(optionNavigation, turn, go);
	attente(600);
	while(askNavigation())
	{
		attente(100);
		//Serial.println(askNavigation());
	}
}

void ASPIRATION_BALLE(int k)
{
	// Récuperation des balles
	arceau.attach(pinArceau,800,2500);
	arceau.writeMicroseconds(arceauBas);
	analogWrite(pinSouffleur,255);
	analogWrite(pinEjecteurBalle,127);
	//attente(2000);

	rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
	rouleauBalle.writeMicroseconds(rouleauAval);
	for (int i=0; i<=k;i++)
	{
		//attente(temps/(k/2));
		TURN_GO(0,false,0,60);
		//attente(temps/(k/2));
		TURN_GO(0,false,0,-60);
	}
	//rouleauBalle.detach();
	analogWrite(pinSouffleur,0);
	arceau.attach(pinArceau,1000,2500);
	arceau.writeMicroseconds(arceauHaut);
	//attente(1000);
}

void EJECTION_BALLE(int temps)
{
	// Ejection des balles
	//rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
	//rouleauBalle.writeMicroseconds(rouleauAval);
	//attente(1000);
	pont.attach(pinPont, 1000, 2350);
	pont.writeMicroseconds(pontBas);
	//attente(1000);
	analogWrite(pinEjecteurBalle,127);
	//attente(1000);
	bascule.attach(pinBascule, 1000, 2000);
	bascule.writeMicroseconds(basculeHaut);
	attente(temps/2);
	bascule.writeMicroseconds(basculeBas);
	attente(300);
	bascule.writeMicroseconds(basculeHaut);
	attente(temps/2);
	rouleauBalle.detach();
	pont.attach(pinPont, 1000, 2350);
	pont.writeMicroseconds(pontHaut);
	//attente(500);
	analogWrite(pinEjecteurBalle,0);
	bascule.attach(pinBascule, 1000, 2000);
	bascule.writeMicroseconds(basculeBas);
	//attente(1000);
}

void REJET_BALLE(int temps)
{
	bascule.writeMicroseconds(basculeBas);
	rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
	rouleauBalle.writeMicroseconds(rouleauRecrache);
	attente(temps);
	rouleauBalle.detach();
	//attente(500);
}

void REJET_BALLE_DEBUT()
{
	rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
	rouleauBalle.writeMicroseconds(rouleauRecrache);
}

void REJET_BALLE_FIN()
{
	rouleauBalle.detach();
}

void initActionneur()
{
	/*
	analogWrite(pinSouffleur,souffleurON);
	delay(2000);
	analogWrite(pinSouffleur,souffleurOFF);

	analogWrite(pinEjecteurBalle,ejecteurON);
	delay(2000);
	analogWrite(pinEjecteurBalle,ejecteurOFF);
	*/
	balise.attach(pinBalise,1000,2000);
	balise.writeMicroseconds(baliseMilieu);
	delay(500);
	arceau.attach(pinArceau,800,2500);
	arceau.writeMicroseconds(arceauHaut);
	delay(500);
	pinceGauche.attach(pinPinceGauche, 1000, 2200);
	pinceGauche.writeMicroseconds(pinceGaucheRangee);
	delay(1000);
	pinceDroite.attach(pinPinceDroite, 1000, 2200);
	pinceDroite.writeMicroseconds(pinceDroiteRangee);
	delay(1000);
	rouleauBalle.attach(pinRouleauBalle, 1000, 2000);
	rouleauBalle.writeMicroseconds(rouleauStop);
	delay(500);
	bascule.attach(pinBascule, 1000, 2000);
	bascule.writeMicroseconds(basculeBas);
	delay(500);
	pont.attach(pinPont, 1000, 2350);
	pont.writeMicroseconds(pontHaut);
	delay(500);

	pinceGauche.detach();
	pinceDroite.detach();
	rouleauBalle.detach();
	bascule.detach();
	pont.detach();
	arceau.detach();
	//balise.detach();
}

bool mouvement(Servo actionneur, int destination, int vitesse)
{
	bool etat ;
	int position = actionneur.readMicroseconds() ;
	int delta = destination - position ;

	char inc = 1 ;

	int signe = 1 ;
	if ( delta < 0 ) signe = -1 ;

	if (actionneur.attached())
	{
		if ( position != destination )
		{
			position = position + (inc * signe);
			if ( ( delta > 0 && position > destination )
			|| ( delta < 0 && position < destination ) )
			{
				position = destination ;
			}
			actionneur.writeMicroseconds(position);
			attente(vitesse);
		}
		etat = true ;
	}
	else
	{
		etat = false ;
	}
	return etat ;
}

void sendIHM(byte fonction)
{
	Wire.beginTransmission(IHM);
	Wire.write(fonction);
	Wire.endTransmission();
}

bool askNavigation()
{
	Wire.requestFrom(NAVIGATION, 1);
	etatNavigation = true ;
	while (Wire.available())
	{
		if (Wire.read()>0) etatNavigation = true ;
		else etatNavigation = false;
	}
	return etatNavigation;
}

void askIHM()
{
	// Demande l'état de l'IHM
	Wire.requestFrom(IHM, 1);
	// Lecture de l'état de l'IHM
	byte command = 0;
	while (Wire.available())
	{
		command = Wire.read();
	}
	for (int i =0;i<4;i++)
	{
		button[i]=bitRead(command,i);
	}
	equipe = button[TEAM];
	// Echo pour vérification
	Wire.beginTransmission(IHM);
	Wire.write('b');
	Wire.write(command);
	Wire.endTransmission();
}

void updateBalise()
{
	baliseA.update();
  if (baliseState == 0)
  {
    if (baliseA.readPosition() == 1200)
    {
		baliseA.setDestination(1600, 250);
    }
    else
    {
		baliseA.setDestination(1200, 250);
    }
    baliseState = 1 ;
  }
  if (baliseState == 1 && !baliseA.update())
  {
      baliseState = 0 ;
  }
}

//----------------MISE A JOUR DU TEMPS DE MATCH----------------
int UPDATE_TEMPS()
{
  int TEMPS_RESTANT = ( TEMPS_MATCH - (millis() - timeInit) ) / 1000;
  if ( TEMPS_RESTANT <= 0 )
  {
    FIN_MATCH();
  }
  return TEMPS_RESTANT;
}

void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		// Faire des choses dans la procedure d'attente
		updateBalise();
		UPDATE_TEMPS();
	}
}


void sendNavigation(byte fonction, int X, int Y, int rot)
{
	Wire.beginTransmission(NAVIGATION);
	Wire.write(fonction);
	Wire.write(X >> 8);
	Wire.write(X & 255);
	Wire.write(Y >> 8);
	Wire.write(Y & 255);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.endTransmission();
}

void sendNavigation(byte fonction, int rot, int dist)
{
	if ( equipe == BLEU ) rot = -rot ;
	Wire.beginTransmission(NAVIGATION);
	Wire.write(fonction);
	Wire.write(rot >> 8);
	Wire.write(rot & 255);
	Wire.write(dist >> 8);
	Wire.write(dist & 255);
	Wire.endTransmission();
}

void FIN_MATCH()
{
	// Stopper les moteurs
	sendNavigation(255, 0, 0);
	// Detacher les servomoteurs
	pinceGauche.detach();
	pinceDroite.detach();
	rouleauBalle.detach();
	pont.detach();
	bascule.detach();
	arceau.detach();
	balise.detach();
	// Boucle infinie
	while(1)
	{
		// Stopper les moteurs
		sendNavigation(255, 0, 0);
		TURN_GO(false,false,0,0);
	}
}
