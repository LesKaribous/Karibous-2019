#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <FastCRC.h>
#include "Actionneur.h"
#include <Servo.h>
//#include <Capteur.h>

// Adresse I2C du module de navigation
#define ADRESSE 80
// Etat des actions
#define FINI      0
#define EN_COURS  1
#define PREVU     2
// Etat de la nouvelle action demandée
#define VALIDEE     0 // Nouvelle action validée et prise en compte
#define DISPONIBLE  1 // Nouvelle action enregistrée
#define ERRONEE     2 // nouvelle action erronée. CRC nok.
// Liste des différentes actions
// Actions sur les bras pour l'abeille
#define BD_HAUT 0
#define BD_BAS  1
#define BG_HAUT 2
#define BG_BAS  3
// Actions de recuperation/envoi des balles
#define RECUP_BALLES_COMPLET 4
#define RECUP_BALLES_SAFE 5
#define RECUP_BALLES_ORANGE 12
#define ENVOI_BALLES 6
// Actions sur la balise
#define GO_BALISE 7
#define STOP_BALISE 8
// Actions sur les cales
#define CALE_BAS 9
#define CALE_HAUT 10
// INIT barillet
#define INIT_BARILLET 11

Servo brasGauche  ;
Servo brasDroit   ;
Servo barriere    ;
Servo balise      ;
Servo selecteur   ;
Servo trappe      ;
Servo caleDroite  ;
Servo caleGauche  ;

// Declaration des broches d'ES pour les satellites
// Broches analogiques :
int ana_1 = A6 ; // 1 - pin 20 ou A6 - PWM
int ana_2 = A7 ; // 2 - pin 21 ou A7 - PWM
int ana_3 = A8 ; // 3 - pin 22 ou A8 - PWM
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
const int hautBrasGauche    = 118   ;
const int basBrasGauche     = 35    ;
const int hautBrasDroit     = 30    ;
const int basBrasDroit      = 115   ;
const int hautBarriere      = 120   ;
const int basBarriere       = 40    ;
const int droiteBalise      = 2100  ;
const int gaucheBalise      = 1000  ;
const int milieuBalise      = 1500  ;
const int hautTrappe        = 180   ;
const int basTrappe         = 95    ;
const int basCaleDroite     = 155   ;
const int hautCaleDroite    = 65    ;
const int basCaleGauche     = 35    ;
const int hautCaleGauche    = 115   ;

const int positionSelecteur[3] = {170,130,40};
const int sequenceBarilletComplet[5] = {-722,279,150,150};
const int sequenceBarilletSafe[3] = {572,143,1200};
const int sequenceBarilletEnvoi[2] = {143,0};
// Variables du moteur de lancé de balles
int moteurBalles = ana_1      ;
const int vitMaxBalles = 60   ;
const int vitMinBalles = 10   ;

bool baliseState = 0;
Actionneur baliseA(balise,ana_4,1000,800,2400);
bool commandeBalise = 0;

// Pin IO pour le moteur pas-à-pas du barillet
int pinStep1=8, pinDir1=9, pinSleep1=7, pinReset1=6;
// Declaration du moteur barillet
AccelStepper MBarillet(AccelStepper::DRIVER,pinStep1, pinDir1);

int initTemps = 0;
bool indexTemps = false;
int nbrBalles = 0;

FastCRC8 CRC8;
byte bufAction[2]={0,0}; // Buffer de reception des ordres d'action + le CRC
byte crcAction = 0; // CRC de controle des ordres d'action'
byte etatAction ;

int capteurBarillet = ana_3;

const float VitesseMaxBarillet = 200.0; //Ancien : 130 11/05/2018
const float VitesseMinBarillet = 50.0; //Ancien :
const float AccelMin = 50.0; //Ancien :
const float AccelMax = 100.0; //Ancien :
const float AccelStop = 2000.0; //Ancien :

int indexAction = 0 ;
int indexAccMoteur = vitMinBalles ;

int16_t actionRequest ; // action

byte newAction = VALIDEE;

void setup();
void loop();
void receiveEvent(int HowMany);
void requestEvent();
void updateAction();
void selectAction();
void finMatch();
void updateBalise();
void executeAction();

void actionBras();
void actionEnvoiBalles();
void actionRecuperationComplet();
void actionRecuperationSafe();
void actionRecuperationOrange();
bool attente(int temps);
bool accelerationMoteur();

void actionCale(bool etatCale);

void initBarillet();
void actionBalise(bool etatActionBalise);
