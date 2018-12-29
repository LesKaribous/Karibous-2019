#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60

//Etat des d�placements
#define FINI 0
#define EN_COURS 1
#define PREVU 2


//Etat de la nouvelle position demand�e
#define VALIDEE 0 // Nouvelle position valid�e et prise en compte
#define DISPONIBLE 1 // Nouvelle position enregistr�e
#define ERRONEE 2 // nouvelle position erron�e. CRC nok.



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
byte bufNavAbsolu[8]={0,0,0,0,0,0,0,0}; // Buffer de reception des ordres de navigation relatifs + le CRC
byte crcNavAbsolu = 0; // CRC de controle des ordres de navigation relatifs



byte fonction ;
int16_t relativeRequest[2] ; // rotation, distance
int16_t absoluteRequest[3] ; // rotation, X, Y
int16_t currentPos[3]; //rotation, X, Y

byte newPos = VALIDEE;

// Declaration des broches d'ES pour les satellites
// Broches analogiques :
int ana_1 = A6 ; // 1 - pin 20 ou A6 - PWM
int ana_2 = 21 ; // 2 - pin 21 ou A7 - PWM
int ana_3 = 22 ; // 3 - pin 22 ou A8 - PWM
int ana_4 = A9 ; // 4 - pin 23 ou A9 - PWM
int ana_5 = A0 ; // 5 - pin 14 ou A0
int ana_6 = A1 ; // 6 - pin 15 ou A1
int ana_7 = A2 ; // 7 - pin 16 ou A2
int ana_8 = A3 ; // 8 - pin 17 ou A3
// Broches numeriques : ( Utilisé par le module de moteurs pas-à-pas )
// int digi_1 = 5 ; // 1 - PWM
// int digi_2 = 4 ; // 2 - PWM
// int digi_3 = 3 ; // 3 - PWM
// int digi_4 = 2 ; // 4
// int digi_5 = 9 ; // 5 - PWM
// int digi_6 = 8 ; // 6
// int digi_7 = 7 ; // 7
// int digi_8 = 6 ; // 8 - PWM

// Declaration des variables liées à la detection d'adversaire
bool presenceArriere = 0, presenceAvant = 0;
bool presenceArriereTemp = 0, presenceAvantTemp = 0;
int adversaireArriere = ana_2;
int adversaireAvant = ana_3;
int angleBalise = ana_1;
int angleAvant = 0, angleArriere = 0;
int seuilAvant = 150,seuilArriere = 150;

double AskX, AskRot, TempGauche, TempDroit, NewX, NewRot ;

int sensorTime = 1000;
int avantTimeInit = 0;
int arriereTimeInit = 0;

bool optionAdversaire = false;
bool optionRecalage = false;
bool optionRalentit = false;

char etatRotation = 'a', etatAvance = 'a';
bool etatABS = false;
bool etatLastRot = false;

int16_t targetRot = 0;

const float FacteurX= 1.09; //Ancien : 154.8
const float FacteurDroit = 8.0; //Ancien : 154.8
const float FacteurGauche = 8.0; //Ancien : 154.8
const float FacteurRot = 4.25; //Ancien : 19.64

const float VitesseMaxDroite = 6000.0; //Ancien : 3000 11/05/2018
const float VitesseMaxGauche = 6000.0; //Ancien : 3000 11/05/2018
const float VitesseMinDroite = 4000.0; //Ancien : 4000 23/06/2018
const float VitesseMinGauche = 4000.0; //Ancien : 4000 23/06/2018
const float AccelRot = 2500.0; //Ancien : 2000
const float AccelMin = 3000.0; //Ancien : 2000
const float AccelMax = 5000.0; //Ancien : 5000
const float AccelStop = 4000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
//int PIN_BORDURE[4] = {20,17,16,21};

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
//bool detection(int capteur, int iteration);
