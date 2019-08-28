#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
//#include <Capteur.h>

//Adresse I2C du module de navigation
#define ADRESSE 60

// Etat des déplacements
#define FINI 0
#define EN_COURS 1
#define PREVU 2

// Type de ROBOT
#define ROBOT_PRIMAIRE 1
#define ROBOT_SECONDAIRE 0

// Etat de la nouvelle position demand�e
#define VALIDEE 0 // Nouvelle position valid�e et prise en compte
#define DISPONIBLE 1 // Nouvelle position enregistr�e
#define ERRONEE 2 // nouvelle position erron�e. CRC nok.

const int16_t centerOffset[2] = {0, 0};

//Variable Pin Moteur
const int pinStep1 = 2;
const int pinDir1 = 3;

const int pinStep2 = 5;
const int pinDir2 = 6;

const int pinSleep = 4;

const int pinM0 = 7;
const int pinM1 = 0;
const int pinM2 = 1;

// Pin du type de robot
const int pinRobot = 17;

// Declaration des objets AccelStepper
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
// int ana_1 = A6 ; // 1 - pin 20 ou A6 - PWM
// int ana_2 = 21 ; // 2 - pin 21 ou A7 - PWM
// int ana_3 = 22 ; // 3 - pin 22 ou A8 - PWM
// int ana_4 = A9 ; // 4 - pin 23 ou A9 - PWM
// int ana_5 = A0 ; // 5 - pin 14 ou A0
// int ana_6 = A1 ; // 6 - pin 15 ou A1
// int ana_7 = A2 ; // 7 - pin 16 ou A2
// int ana_8 = A3 ; // 8 - pin 17 ou A3
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
int adversaireArriere = 11;
int adversaireAvant = 12;
int angleBalise = A3;
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

bool typeRobot = ROBOT_PRIMAIRE ;

int16_t targetRot = 0;

// Variable par defaut pour le réglage des déplacements
float FacteurX= 1.03; //Ancien : 154.8
float FacteurDroit = 8.0; //Ancien : 154.8
float FacteurGauche = 8.0; //Ancien : 154.8
float FacteurRot = 3.640; // Secondaire : 3.655 | Primaire : 3.600

float VitesseMaxDroite = 4000.0; //Ancien : 3000 11/05/2018
float VitesseMaxGauche = 4000.0; //Ancien : 3000 11/05/2018
float VitesseMinDroite = 2000.0; //Ancien : 4000 23/06/2018
float VitesseMinGauche = 2000.0; //Ancien : 4000 23/06/2018
float AccelRot = 2500.0; //Ancien : 2000
float AccelMinDroite = 3000.0; //Ancien : 2000
float AccelMinGauche = 3000.0; //Ancien : 2000
float AccelMaxDroite = 5000.0; //Ancien : 5000
float AccelMaxGauche = 5000.0; //Ancien : 5000
float AccelStop = 4000.0; //Ancien : 8000

// ----------Paramètres selon type de robot-----------

// --SECONDAIRE--
const float secondaireFacteurX= 1.03; //Ancien : 154.8
const float secondaireFacteurDroit = 8.090; //Ancien : 154.8
const float secondaireFacteurGauche = 8.064; //Ancien : 154.8
const float secondaireFacteurRot = 13.359; // Ancien : 3.655

const float secondaireVitesseMaxDroite = 3000.0; //Ancien : 3000 11/05/2018
const float secondaireVitesseMaxGauche = 2840.0; //Ancien : 3000 11/05/2018
const float secondaireVitesseMinDroite = 1500.0; //Ancien : 4000 23/06/2018
const float secondaireVitesseMinGauche = 1450.0; //Ancien : 4000 23/06/2018

const float secondaireAccelRot = 2000.0; //Ancien : 2000

const float secondaireAccelMaxDroite = 3000.0; //Ancien : 5000
const float secondaireAccelMaxGauche = 2840.0; //Ancien : 5000
const float secondaireAccelMinDroite = 1500.0; //Ancien : 2000
const float secondaireAccelMinGauche = 1450.0; //Ancien : 2000


const float secondaireAccelStop = 6000.0; //Ancien : 8000

// --PRIMAIRE--
const float primaireFacteurX= 1.03; //Ancien : 154.8
const float primaireFacteurDroit = 8.064; //Ancien : 8.064
const float primaireFacteurGauche = 8.090; //Ancien : 8.128
const float primaireFacteurRot = 13.149; // Ancien : 3.600

const float facteurPrimaireMax = 3.0 ;
const float facteurPrimaireAccelMax = 2.0 ;
const float facteurPrimaireMin = 1.0 ;
const float facteurPrimaireAccelMin = 1.0 ;
const float facteurPrimaireAccelRot = 1.0 ;

const float primaireVitesseMaxDroite = 2840.0 * facteurPrimaireMax; //Ancien : 2963.253
const float primaireVitesseMaxGauche = 3000.0 * facteurPrimaireMax; //Ancien : 3000 11/05/2018
const float primaireVitesseMinDroite = 1450.0 * facteurPrimaireMin; //Ancien : 1481.627
const float primaireVitesseMinGauche = 1500.0 * facteurPrimaireMin; //Ancien : 1500

const float primaireAccelRot = 2000.0 * facteurPrimaireAccelRot; //Ancien : 2000

const float primaireAccelMaxDroite = 2840.0 * facteurPrimaireAccelMax; //Ancien : 2963.253
const float primaireAccelMaxGauche = 3000.0 * facteurPrimaireAccelMax; //Ancien : 3000
const float primaireAccelMinDroite = 1450.0 * facteurPrimaireAccelMin; //Ancien : 1975.502
const float primaireAccelMinGauche = 1500.0 * facteurPrimaireAccelMin; //Ancien : 2000


const float primaireAccelStop = 6000.0; //Ancien : 8000

byte BORDURE = 0 ;
// AV_DROIT , AV_GAUCHE , AR_DROIT , AR_GAUCHE
int PIN_BORDURE[4] = {13,14,15,16};
void debug();

void updatePos();

void turnGo();

void goTo();

void recalage();

void bordure();

void adversaire();

void changeTypeRobot(bool type);

//Fin de match
void FIN_MATCH();

void receiveEvent(int howMany);
void requestEvent();
//bool detection(int capteur, int iteration);
