#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60

//Etat des déplacements
#define FINI 0
#define EN_COURS 1
#define PREVU 2


//Etat de la nouvelle position demandée
#define VALIDEE 0 // Nouvelle position validée et prise en compte
#define DISPONIBLE 1 // Nouvelle position enregistrée
#define ERRONEE 2 // nouvelle position erronée. CRC nok.



const int16_t centerOffset[2] = {0, 0};

//Variable Moteur
const int pinStep1=4;
const int pinDir1=5;
const int pinSleep1=3;
const int pinReset1=2;

const int pinStep2=8;
const int pinDir2=9;
const int pinSleep2=7;
const int pinReset2=6;


AccelStepper MGauche(AccelStepper::DRIVER,pinStep1, pinDir1);
AccelStepper MDroit(AccelStepper::DRIVER,pinStep2, pinDir2);


//Variable I2C
FastCRC8 CRC8;
byte bufNavRelatif[6]={0,0,0,0,0,0}; // Buffer de reception des ordres de navigation relatifs + le CRC
byte crcNavRelatif = 0; // CRC de controle des ordres de navigation relatifs



byte fonction ;
int16_t relativeRequest[2] ; // rotation, distance
int16_t absoluteRequest[3] ; // rotation, X, Y
int16_t currentPos[3]; //rotation, X, Y 

byte newPos = VALIDEE;
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
bool etatABS = false;
bool etatLastRot = false;

int16_t targetRot = 0;




const float FacteurX= 2.17; //Ancien : 154.8
const float FacteurDroit = 8.0; //Ancien : 154.8
const float FacteurGauche = 8.0; //Ancien : 154.8
const float FacteurRot = 6.17; //Ancien : 19.64

const float VitesseMaxDroite = 4500.0; //Ancien : 8000
const float VitesseMaxGauche = 4500.0; //Ancien : 8000
const float VitesseMinDroite = 2000.0; //Ancien : 5000
const float VitesseMinGauche = 2000.0; //Ancien : 5000
const float AccelRot = 2000.0; //Ancien : 2000
const float AccelMin = 1000.0; //Ancien : 2000
const float AccelMax = 2000.0; //Ancien : 5000
const float AccelStop = 4000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
//int PIN_BORDURE[4] = {20,17,16,21};



void setup;

void loop();

void updatePos();

void turnGo();

void goTo();

void recalage();

void bordure();

void adversaire();

//Fin de match
void FIN_MATCH();

void receiveEvent(int howMany);
void requestEvent();
