#include "Strategie.h"

void setup()
{
  pinMode(pinTirette,INPUT_PULLUP);
  pinMode(pinDetection,INPUT_PULLUP);
  pinMode(pinStrategie,INPUT_PULLUP);
  pinMode(pinRobot,INPUT_PULLUP);

  Serial.begin(9600);     //Demarrage d'un liaison série pour le debug
  Wire.begin();           //Demarrage de la liaison I2C

  delay(500);             //Pause de demarrage avant lancement

  u8g2.begin();           //Init du LCD
  u8g2_splash_screen();   //Affichage du Logo des Karibous
  initActionneur();       //Initialisation des actionneurs
  delay(1000);            //Attente affichage logo

  // Gestion tirette
  while (digitalRead(pinTirette))
  {
    //Menu d'avant Match
    bouttonIHM();
    u8g2_menu_avant_match();
  }
  while (!digitalRead(pinTirette))
  {
    //Menu d'avant Match
    bouttonIHM();
    u8g2_menu_avant_match();
  }

  // Lancement du Match
	timeInit = millis();
  u8g2_splash_screen_GO();
	delay (100);
  digitalWrite(pinBalise,HIGH);
}

void loop()
{
  if (typeRobot == ROBOT_PRIMAIRE)
  {
    if (strategie == STRATEGIE_HOMOLOGATION) testActionneur(); //testLigneDroite(); //homologationPrimaire();
    else matchPrimaire(); // Match
  }
  else
  {
    if (strategie == STRATEGIE_HOMOLOGATION) demoRotation(); //testRotation(); // homologationSecondaire();
    else matchSecondaire(); // Match
  }
}

//----------------INIT ACTIONNEUR-------------
void initActionneur()
{
  // Check du type de robot :
  typeRobot = digitalRead(pinRobot) ;
  // Initialisation des actionneurs en fonction du type de robot
    // Detacher les servos par securité

    servoGauche.detach();
    servoDroit.detach();
    servoBrasDroit.detach();
    servoBrasGauche.detach();
    servoAvant.detach();

    if (typeRobot == ROBOT_PRIMAIRE)
    {
      // Si strategie pour le robot primaire selectionné :
      pinMode(pinPompeGauche,OUTPUT);
      digitalWrite(pinPompeGauche,HIGH);

      pinMode(pinEVGauche,OUTPUT);
      digitalWrite(pinEVGauche,HIGH);

      pinMode(pinPompeDroit,OUTPUT);
      digitalWrite(pinPompeDroit,HIGH);

      pinMode(pinEVDroit,OUTPUT);
      digitalWrite(pinEVDroit,HIGH);

      pinBalise = pinBalisePrimaire ;
      pinMode(pinBalise, OUTPUT);
      digitalWrite(pinBalise,LOW);

      servoGauche.attach(pinServoGauche);
      servoDroit.attach(pinServoDroit);
      servoGauche.write(sgHaut);
      servoDroit.write(sdHaut);
      delay(1500);
      servoGauche.write(sgBas);
      servoDroit.write(sdBas);
      delay(1500);
      servoGauche.write(sgHaut);
      servoDroit.write(sdHaut);
      digitalWrite(pinPompeGauche,LOW);
      digitalWrite(pinPompeDroit,LOW);
    }
    else
    {
      // Si strategie pour le robot secondaire selectionné :
      pinMode(pinPompeAvant,OUTPUT);
      digitalWrite(pinPompeAvant,HIGH);

      pinMode(pinEVAvant,OUTPUT);
      digitalWrite(pinEVAvant,HIGH);

      pinBalise = pinBaliseSecondaire ;
      pinMode(pinBalise, OUTPUT);
      digitalWrite(pinBalise,LOW);

      servoBrasGauche.attach(pinServoBrasGauche);
      servoBrasDroit.attach(pinServoBrasDroit);
      servoAvant.attach(pinServoAvant);
      servoBrasGauche.write(sgBas_bras);
      servoBrasDroit.write(sdBas_bras);
      servoAvant.write(avBas);
      digitalWrite(pinPompeAvant,LOW);
      delay(2000);
      digitalWrite(pinEVAvant,LOW);
      servoBrasGauche.write(sgHaut_bras);
      servoBrasDroit.write(sdHaut_bras);
      servoAvant.write(avHaut);
      digitalWrite(pinPompeAvant,HIGH);
      digitalWrite(pinEVAvant,HIGH);
    }
}

//----------------GESTION DES BOUTTONS DE L'IHM----------------
void bouttonIHM()
{
  detection = digitalRead(pinDetection) ;
  tirette   = digitalRead(pinTirette)   ;
  strategie = digitalRead(pinStrategie) ;
  typeRobot = digitalRead(pinRobot) ;

  if(analogRead(pinEquipe)>10) equipe = false ;
  else equipe = true ;

  if(analogRead(pinCheck)>10)
  {
    delay(250);
    if(analogRead(pinCheck)>10) check = true ;
  }
  else check = false ;

  if(!check) initRobot();
}

//----------------STRATEGIES----------------
void testLigneDroite()
{
  turnGo(ADVERSAIRE_NON,false,false,0,1000);
  delay(500);
  // turnGo(ADVERSAIRE_NON,false,false,0,-1000);
  // delay(500);
  // turnGo(ADVERSAIRE_NON,false,false,0,600);
  // delay(500);
  // turnGo(ADVERSAIRE_NON,false,false,180,1000);
  // delay(500);
  // turnGo(ADVERSAIRE_NON,false,false,180,0);
  // delay(500);
  while(1);
  finMatch();
}

void testRotation()
{
  turnGo(ADVERSAIRE_NON,false,false,-360*10,0);
  delay(500);
  finMatch();
}

void homologationPrimaire()
{
  turnGo(ADVERSAIRE_OUI,false,true,0,575);
  turnGo(ADVERSAIRE_OUI,false,true,90,780);
  //Ventousage palets
  servoGauche.write(sgHaut-10);
  servoDroit.write(sdHaut+10);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinPompeDroit,LOW);
  attente(800);
  //Retour sur la case bleu
  turnGo(ADVERSAIRE_OUI,false,true,0,-1000);
  turnGo(ADVERSAIRE_OUI,false,true,90,600);
  //Depose palets
  servoGauche.write(sgBas);
  servoDroit.write(sdBas);
  attente(500);
  digitalWrite(pinPompeGauche,HIGH);
  digitalWrite(pinPompeDroit,HIGH);
  digitalWrite(pinEVGauche,LOW);
  digitalWrite(pinEVDroit,LOW);
  attente(500);
  digitalWrite(pinEVGauche,HIGH);
  digitalWrite(pinEVDroit,HIGH);
  servoGauche.write(sgHaut);
  servoDroit.write(sdHaut);
  //Recul
  turnGo(ADVERSAIRE_OUI,false,true,0,-150);
  turnGo(ADVERSAIRE_NON,false,true,-90,0);
  //turnGo(ADVERSAIRE_NON,0,false,0,-600);
  finMatch();
}

void homologationSecondaire()
{
  turnGo(ADVERSAIRE_OUI,false,false,0,1250);
  turnGo(ADVERSAIRE_NON,true,true,102,-200);
  turnGo(ADVERSAIRE_NON,false,true,0,-20);
  turnGo(ADVERSAIRE_OUI,false,true,0,80);
  //Ouverture Bras Gauche
  turnGo(ADVERSAIRE_OUI,false,true,-90,0);
  if(equipe==EQUIPE_JAUNE) servoBrasGauche.write(sgBas_bras);
  else servoBrasDroit.write(sdBas_bras);
  turnGo(ADVERSAIRE_OUI,false,true,0,150);
  if(equipe==EQUIPE_JAUNE) servoBrasGauche.write(sgHaut_bras);
  else servoBrasDroit.write(sdHaut_bras);
  servoAvant.write(avHaut-5);
  turnGo(ADVERSAIRE_OUI,false,true,0,530);
  digitalWrite(pinPompeAvant,LOW);
  turnGo(ADVERSAIRE_OUI,false,true,-90,100);
  turnGo(ADVERSAIRE_OUI,false,true,0,-100);
  servoAvant.write(avHaut+5);
  /*
  turnGo(ADVERSAIRE_OUI,false,false,-100,1800);

  servoAvant.write(avBas);
  attente(1000);
  digitalWrite(pinPompeAvant,HIGH);
  digitalWrite(pinEVAvant,HIGH);
  attente(1000);
  servoAvant.write(avHaut);
  digitalWrite(pinEVAvant,LOW);
  */
  finMatch();
}

void sequenceRecalage()
{
  bool recalage = true;
  //Recalage
  if (typeRobot == ROBOT_PRIMAIRE)
  {
    turnGo(ADVERSAIRE_NON,recalage,true,0,-250);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    turnGo(ADVERSAIRE_NON,false,true,0,100);
    delay(500);
    turnGo(ADVERSAIRE_NON,recalage,true,90,-1000);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    delay(500);
    turnGo(ADVERSAIRE_NON,false,true,0,650);          // Centre du robot à 743
    turnGo(ADVERSAIRE_NON,recalage,true,-90,-250);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    turnGo(ADVERSAIRE_NON,false,true,0,160);
    turnGo(ADVERSAIRE_NON,false,true,90,0);
  }
  else
  {
    turnGo(ADVERSAIRE_NON,recalage,true,0,-250);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    turnGo(ADVERSAIRE_NON,false,true,0,100);
    delay(500);
    turnGo(ADVERSAIRE_NON,recalage,true,90,-1000);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    delay(500);
    turnGo(ADVERSAIRE_NON,false,true,0,350);
    turnGo(ADVERSAIRE_NON,recalage,true,-90,-250);
    turnGo(ADVERSAIRE_NON,false,true,0,-20);
    turnGo(ADVERSAIRE_NON,false,true,0,250);
    turnGo(ADVERSAIRE_NON,false,true,-12,0);
    //turnGo(ADVERSAIRE_NON,0,false,0,-600);
  }
}

void matchPrimaire()
{
  //--------Petit distributeur--------
  turnGo(ADVERSAIRE_NON,false,false,0,1183);     // Avancer jusqu'au distributeur
  // Ventousage palets
  servoGauche.write(sgHaut-12);
  servoDroit.write(sdHaut+12);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinPompeDroit,LOW);
  attente(800);
  servoGauche.write(sgHaut);
  servoDroit.write(sdHaut);
  attente(200);
  // Desengagement bordure
  turnGo(ADVERSAIRE_NON,false,false,0,-300);
  // Rotation et avance pour dépose du palet rouge dans la zone
  turnGo(ADVERSAIRE_NON,false,false,180,1087);
  // Depose du palet rouge
  if(equipe==EQUIPE_JAUNE)
  {
    servoGauche.write(sgBas);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  else
  {
    servoDroit.write(sdBas);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  majScore(6, 1);
  // depose du palet vert
  turnGo(ADVERSAIRE_NON,false,false,0,-300);
  // Depose du palet vert
  if(equipe==EQUIPE_VIOLET)
  {
    servoGauche.write(sgBas);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  else
  {
    servoDroit.write(sdBas);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  majScore(6, 1);
  // Recul pour desengagement
  turnGo(ADVERSAIRE_NON,false,false,0,-300);
  // Recalage
  turnGo(ADVERSAIRE_NON,false,true,90,-240);
  // Desengagement bordure
  turnGo(ADVERSAIRE_NON,false,false,0,65);
  // Avancer jusqu'au distributeur
  turnGo(ADVERSAIRE_NON,false,false,90,875);
  // Ventousage palet bleu
  if(equipe==EQUIPE_JAUNE)
  {
    servoDroit.write(sdHaut+12);
    digitalWrite(pinPompeDroit,LOW);
    attente(800);
  }
  else
  {
    servoGauche.write(sgHaut-12);
    digitalWrite(pinPompeGauche,LOW);
    attente(800);
  }
  // Desengagement bordure
  turnGo(ADVERSAIRE_NON,false,false,0,-300);
  // Rotation et avance pour dépose du palet bleu dans la zone
  turnGo(ADVERSAIRE_NON,false,false,180,487);
  // Depose du palet bleu
  if(equipe==EQUIPE_JAUNE)
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  else
  {
    servoGauche.write(sgMilieu);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  majScore(6, 1);
  // Desengagement zone bleu
  turnGo(ADVERSAIRE_NON,false,false,0,-150);
  // Recalage
  turnGo(ADVERSAIRE_NON,false,false,90,-130);
  //--------Grand distributeur--------
  //-------------- 1 -----------------
  // Avance jusqu'au grand distributeur
  turnGo(ADVERSAIRE_NON,false,false,0,440);
  // Tourne et prise des palets
  turnGo(ADVERSAIRE_NON,false,false,90,300);
  // Ventousage palets
  servoGauche.write(sgHaut-12);
  servoDroit.write(sdHaut+12);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinPompeDroit,LOW);
  attente(800);
  servoGauche.write(sgHaut);
  servoDroit.write(sdHaut);
  attente(200);
  // Desengagement distributeur
  turnGo(ADVERSAIRE_NON,false,false,0,-120);
  // Rotation et positionnement devant la prochaine zone
  turnGo(ADVERSAIRE_OUI,false,false,180,910);
  // Positionnement palet Rouge
  turnGo(ADVERSAIRE_NON,false,false,-90,250);
  // Depose palet rouge
  if(equipe==EQUIPE_JAUNE)
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  else
  {
    servoGauche.write(sgMilieu);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  majScore(6, 1);
  // Desengagement zone Rouge
  turnGo(ADVERSAIRE_OUI,false,false,0,-250);
  // Positionnement seconde zone
  turnGo(ADVERSAIRE_OUI,false,false,-90,300);
  // Positionnement zone verte
  turnGo(ADVERSAIRE_NON,false,false,90,250);
  // Depose du palet vert
  if(equipe==EQUIPE_JAUNE)
  {
    servoGauche.write(sgMilieu);
    //attente(300);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  else
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  majScore(6, 1);
  // Desengagement vert
  turnGo(ADVERSAIRE_OUI,false,false,0,-465);
  //--------Grand distributeur--------
  //-------------- 2 -----------------
  // Ventousage palets suivants
  turnGo(ADVERSAIRE_OUI,false,false,-90,795);
  // Ventousage palets
  servoGauche.write(sgHaut-12);
  servoDroit.write(sdHaut+12);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinPompeDroit,LOW);
  attente(800);
  servoGauche.write(sgHaut);
  servoDroit.write(sdHaut);
  attente(200);
  // Desengagement distributeur
  turnGo(ADVERSAIRE_OUI,false,false,0,-120);
  // Rotation et positionnement devant la prochaine zone
  turnGo(ADVERSAIRE_OUI,false,false,180,910);
  // Positionnement palet Rouge
  turnGo(ADVERSAIRE_NON,false,false,-90,400);
  // Depose palet rouge
  if(equipe==EQUIPE_JAUNE)
  {
    servoDroit.write(sdMilieu);
    //attente(300);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  else
  {
    servoGauche.write(sgMilieu);
    //attente(300);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  majScore(6, 1);
  // Desengagement zone Rouge
  turnGo(ADVERSAIRE_OUI,false,false,0,-200);
  // Positionnement seconde zone
  turnGo(ADVERSAIRE_OUI,false,false,-90,600);
  // Positionnement zone bleu
  turnGo(ADVERSAIRE_NON,false,false,90,200);
  // Depose du palet bleu
  if(equipe==EQUIPE_JAUNE)
  {
    servoGauche.write(sgMilieu);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  else
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  majScore(6, 1);
  // Desengagement zone bleu
  turnGo(ADVERSAIRE_OUI,false,false,0,-590);
  //--------Grand distributeur--------
  //-------------- 3 -----------------
  // Ventousage palets suivants
  turnGo(ADVERSAIRE_OUI,false,false,-90,455);
  // Ventousage palets
  servoGauche.write(sgHaut-12);
  servoDroit.write(sdHaut+12);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinPompeDroit,LOW);
  attente(800);
  servoGauche.write(sgHaut);
  servoDroit.write(sdHaut);
  attente(200);
  // Desengagement distributeur
  turnGo(ADVERSAIRE_OUI,false,false,0,-120);
  // Rotation et positionnement devant la prochaine zone
  turnGo(ADVERSAIRE_OUI,false,false,180,910);
  // Positionnement palet Rouge
  turnGo(ADVERSAIRE_NON,false,false,-90,600);
  // Depose palet rouge
  if(equipe==EQUIPE_JAUNE)
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  else
  {
    servoGauche.write(sgMilieu);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  majScore(6, 1);
  // Desengagement zone Rouge
  turnGo(ADVERSAIRE_OUI,false,false,0,-200);
  // Positionnement seconde zone
  turnGo(ADVERSAIRE_OUI,false,false,-90,300);
  // Positionnement zone verte
  turnGo(ADVERSAIRE_NON,false,false,90,250);
  // Depose du palet vert
  if(equipe==EQUIPE_JAUNE)
  {
    servoGauche.write(sgMilieu);
    //attente(500);
    digitalWrite(pinPompeGauche,HIGH);
    digitalWrite(pinEVGauche,LOW);
    attente(500);
    digitalWrite(pinEVGauche,HIGH);
    servoGauche.write(sgHaut);
  }
  else
  {
    servoDroit.write(sdMilieu);
    //attente(500);
    digitalWrite(pinPompeDroit,HIGH);
    digitalWrite(pinEVDroit,LOW);
    attente(500);
    digitalWrite(pinEVDroit,HIGH);
    servoDroit.write(sdHaut);
  }
  majScore(6, 1);
  // Desengagement vert
  turnGo(ADVERSAIRE_OUI,false,false,0,-450);



  finMatch();
}

void matchSecondaire()
{
  // Match secondaire
  // Desengagement zone de départ
  turnGo(ADVERSAIRE_OUI,false,false,0,1250);
  majScore(40, 1); // Depose, activation de l'experience et arrivée de l'electron
  //Recalage bordure accelerateur
  turnGo(ADVERSAIRE_NON,true,false,102,-200);
  // Recalage bordure
  turnGo(ADVERSAIRE_NON,false,true,0,-20);
  // Desengagement bordure
  turnGo(ADVERSAIRE_OUI,false,false,0,75);
  //Ouverture Bras Gauche
  turnGo(ADVERSAIRE_OUI,false,true,-90,0);
  if(equipe==EQUIPE_JAUNE) servoBrasGauche.write(sgBas_bras);
  else servoBrasDroit.write(sdBas_bras);
  // Poussage du palet dans l'accelerateur
  turnGo(ADVERSAIRE_OUI,false,true,0,160);
  majScore(10, 1) ; // 10 points supplémentaires lorsque le détecteur a été déverrouillé (le "Goldenium" à été révélé)
  majScore(10, 1) ; // 10 points pour chaque atome présent dans l’accélérateur de particules
  if(equipe==EQUIPE_JAUNE) servoBrasGauche.write(sgHaut_bras);
  else servoBrasDroit.write(sdHaut_bras);
  servoAvant.write(avHaut-5);
  // Positionnement devant le goldenium
  turnGo(ADVERSAIRE_OUI,false,true,0,500);
  // Activation pompe
  digitalWrite(pinPompeAvant,LOW);
  // Prise du Goldenium
  turnGo(ADVERSAIRE_OUI,false,true,-90,100);
  // Desengagement Goldenium
  turnGo(ADVERSAIRE_OUI,false,true,0,-100);
  servoAvant.write(avHaut+5);
  majScore(20, 1); // 20 points supplémentaires si l’atome de "Goldenium" à été extrait du détecteur
  // Rotation pour Recalage
  turnGo(ADVERSAIRE_OUI,false,false,-90,890);
  // Recalage
  turnGo(ADVERSAIRE_NON,true,false,-90,-160);
  // Recalage
  turnGo(ADVERSAIRE_NON,false,false,0,-20);
  turnGo(ADVERSAIRE_OUI,false,false,0,40);
  //turnGo(ADVERSAIRE_OUI,false,false,2,0);
  // Avvance jusqu'à la balance
  turnGo(ADVERSAIRE_OUI,false,true,3,1600);
  // // Rotation Et positionnement aucentre de la table en face de la balance
  // turnGo(ADVERSAIRE_OUI,false,true,51,-1300);
  // // Rotation en face de la balance
  // turnGo(ADVERSAIRE_NON,false,true,141,360);
  // // Depose du goldenium

  servoAvant.write(avQuart);
  attente(1000);
  digitalWrite(pinPompeAvant,HIGH);
  digitalWrite(pinEVAvant,HIGH);
  attente(1000);
  servoAvant.write(avHaut);
  digitalWrite(pinEVAvant,LOW);
  majScore(24, 1); // Dépose du goldenium dans la balance
  attente(1000);
  // Fin de match
  finMatch();
}

void demoRotation()
{
  // Rotation
  for(int i=0;i<=10;i++)
  {
    turnGo(ADVERSAIRE_OUI,false,true,3600,0);
  }
  // Fin de match
  finMatch();
}

void testActionneur()
{
  turnGo(ADVERSAIRE_NON,false,true,0,40); // Avancer vers le bord
  turnGo(ADVERSAIRE_NON,false,true,0,-30); // Avancer vers le bord
  //---------------------------------------
  // Prendre le gobelet
  servoGauche.write(sgBas);
  digitalWrite(pinPompeGauche,HIGH);
    attente(2000);
  servoDroit.write(sdBas);
    attente(800);
  servoGauche.write(sgHaut);
    attente(2000);
  //--------------------------------------
  turnGo(ADVERSAIRE_NON,false,true,0,-80);
  turnGo(ADVERSAIRE_NON,false,true,90,250);
  //---------------------------------------
  // Poser le gobelet
  servoGauche.write(sgBas);
    attente(2000);
  digitalWrite(pinPompeGauche,LOW);
  digitalWrite(pinEVGauche,HIGH);
    attente(1000);
  servoDroit.write(sdHaut);
  digitalWrite(pinEVGauche,LOW);
    attente(1000);
  servoGauche.write(sgHaut);
    attente(2000);
}

//----------------PROCEDURE D'ATTENTE----------------
void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		majTemps();
		u8g2_menu_pendant_match();
	}
}

//----------------GESTION DES PAGES LCD-------------------
void u8g2_prepare() {
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void u8g2_splash_screen() {
  u8g2.clearBuffer();
  u8g2.drawXBMP( 0, 15, LOGO_KARIBOUS_width, LOGO_KARIBOUS_height, LOGO_KARIBOUS_bits);
  u8g2.sendBuffer();
}

void u8g2_splash_screen_GO() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.drawStr( 18, 2, "GO!");
  u8g2.sendBuffer();
}

void u8g2_menu_avant_match() {
  const int ligneDebut = 10;
  const int colonne1 = 14;
  const int colonne2 = 70;

  u8g2.clearBuffer();
  u8g2_prepare();

  u8g2.setFont(u8g2_font_4x6_tf);
  // Affichages des titres :
  u8g2.drawStr( colonne1, ligneDebut,    "      EQUIPE");
  u8g2.drawStr( colonne1, ligneDebut+10, "   EVITEMENT");
  u8g2.drawStr( colonne1, ligneDebut+20, "       ROBOT");
  u8g2.drawStr( colonne1, ligneDebut+30, "ETAT TIRETTE");
  u8g2.drawStr( colonne1, ligneDebut+40, "   STRATEGIE");
  // Ligne de séparation
  u8g2.drawBox(colonne2-4,ligneDebut,1,ligneDebut+37);

  // Etat equipe :
  u8g2.setCursor(colonne2,ligneDebut);
  if ( equipe == EQUIPE_JAUNE ) u8g2.print("JAUNE");
  else u8g2.print("VIOLET");
  // Etat detection:
  u8g2.setCursor(colonne2,ligneDebut+10);
  if ( detection ) u8g2.print("SIMPLE");
  else u8g2.print("COMPLET");
  // Etat type de robot :
  u8g2.setCursor(colonne2,ligneDebut+20);
  if ( typeRobot == ROBOT_PRIMAIRE ) u8g2.print("PRIMAIRE");
  else u8g2.print("SECONDAIRE");
  // Etat tirette :
  u8g2.setCursor(colonne2,ligneDebut+30);
  if ( tirette ) u8g2.print("ATTENTE");
  else u8g2.print("OK");
  // Etat strategie :
  u8g2.setCursor(colonne2,ligneDebut+40);
  if ( strategie ) u8g2.print("HOMOLOGATION");
  else u8g2.print("MATCH");

  u8g2.sendBuffer();
}

void u8g2_menu_pendant_match() {
	u8g2.clearBuffer();
	u8g2_prepare();
  u8g2.setFont(u8g2_font_inr42_mn);
  u8g2.setCursor(8, 9);
  u8g2.print(score);
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr( 0, 0, "Score:");
  u8g2.drawStr( 68, 0, "Temps:      sec");
  u8g2.setCursor(93, 0);
  u8g2.print(tempsRestant);
  u8g2.drawStr( 105, 57, "points");
  u8g2.drawStr( 0, 57, "NOK:");
  u8g2.setCursor(20, 57);
  u8g2.print(nbrBadCRC);
	u8g2.sendBuffer();
}

void majScore(int points, int multiplicateur){
	score = score + (points*multiplicateur);
}

//----------------INITIALISATION DU ROBOT----------------
void initRobot()
{
  //-------Affichage Init-------
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.drawStr( 10, 2, "Init");
  u8g2.sendBuffer();
  attente(1000);
  //-------Recalage bordure-------
  sequenceRecalage();
  //-------Checklist-------
  // A FINIR !
  /*
  for(int i=0;i<6;i++)
  {
    int x = 128;    //On commence le texte à droite
    do
    {
      u8g2.clearBuffer();

      u8g2.drawBox(22,0,2,33);          //Ligne de séparation numéro
      for(int j=0;j<=128;j=j+4) u8g2.drawBox(j,40,2,1);          //Ligne de séparation texte - tirets

      u8g2.setFont(u8g2_font_logisoso32_tn); //Changer la font pour le numero
      u8g2.setCursor(0,0);
      u8g2.print(i+1);                        //Afficher le numero de l'action

      u8g2.setFont(u8g2_font_courB08_tf);       //Changer la font pour la description
      u8g2.drawStr(28, 0, "Action :");     //Afficher le titre de l'action

      u8g2.setFont(u8g2_font_logisoso22_tf);       //Changer la font pour la description
      u8g2.drawStr(25, 10, titreList[i]);     //Afficher le titre de l'action

      u8g2.setFont(u8g2_font_courB08_tf);  //Changer la font pour la description
      u8g2.drawStr(x, 48, list[i]);           //Afficher l'action

      u8g2.sendBuffer();
      attente(100);
      x -= 10 ;                               //Scrolling
    } while(analogRead(pinCheck)>10);
    attente(1000);
  }
  */
}

//----------------DEMANDE L'ETAT DU DEPLACEMENT----------------
int askNavigation()
{
  int etatNavigation ;
  char reponseNavigation ;
  Wire.requestFrom(CARTE_DEPLACEMENT, 1);
  while(Wire.available())
  {
    reponseNavigation = Wire.read();
  }
  // Serial.print("repNav:");
  // Serial.println(reponseNavigation);
  if (reponseNavigation=='N') etatNavigation = RECU ;
  else if (reponseNavigation=='O') etatNavigation = TERMINEE ;
  else if (reponseNavigation=='E') etatNavigation = ERRONEE ;
  else reponseNavigation = BIZARRE ;
	return etatNavigation;
}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT ABSOLU----------------
void sendNavigation(byte fonction, int X, int Y, int rot)
{
	// Stockage des valeurs à envoyer dans le buffer
  bufNavAbsolu[0]=fonction;
	bufNavAbsolu[1]=rot >> 8;
	bufNavAbsolu[2]=rot & 255;
	bufNavAbsolu[3]=X >> 8;
	bufNavAbsolu[4]=X & 255;
	bufNavAbsolu[5]=Y >> 8;
	bufNavAbsolu[6]=Y & 255;

	// Calcul du CRC
	crcNavRelatif = CRC8.smbus(bufNavAbsolu, sizeof(bufNavAbsolu));
	//Serial.println(crcNavRelatif);

	// Envoi des données
	Wire.beginTransmission(CARTE_DEPLACEMENT);
	for(int i=0;i<=6;i++)
	{
		Wire.write(bufNavRelatif[i]);
	}
	//Wire.write(crcNavRelatif);
	Wire.endTransmission();

}

//----------------ENVOI UNE COMMANDE DE DEPLACEMENT RELATIF----------------
void sendNavigation(byte fonction, int rot, int dist)
{
	if ( equipe == EQUIPE_VIOLET ) rot = -rot ;
	// Stockage des valeurs à envoyer dans le buffer
	bufNavRelatif[0]=fonction;
	bufNavRelatif[1]=rot >> 8;
	bufNavRelatif[2]=rot & 255;
	bufNavRelatif[3]=dist >> 8;
	bufNavRelatif[4]=dist & 255;
	// Calcul du CRC
	crcNavRelatif = CRC8.smbus(bufNavRelatif, sizeof(bufNavRelatif));
	//Serial.println(crcNavRelatif);
	// Envoi des données
	Wire.beginTransmission(CARTE_DEPLACEMENT);
	for(int i=0;i<=4;i++)
	{
		Wire.write(bufNavRelatif[i]);
	}
	Wire.write(crcNavRelatif);
	Wire.endTransmission();
}

//----------------ENVOI UNE COMMANDE TURN GO----------------
void turnGo(bool adversaire, bool recalage,bool ralentit,int turn, int go)
{
  int reponseNavigation ;
  bool optionDetection = detection || adversaire; //

	bitWrite(optionNavigation,0,optionDetection); // false -> la detection adverse est active
	bitWrite(optionNavigation,1,recalage);
	bitWrite(optionNavigation,2,ralentit);
	sendNavigation(optionNavigation, turn, go);
	//attente(100);
  reponseNavigation = askNavigation();
	while(reponseNavigation!=TERMINEE)
	{
    if (reponseNavigation==ERRONEE)
    {
      sendNavigation(optionNavigation, turn, go);
      nbrBadCRC++;
      // Serial.print("nbrBadCRC:");
      // Serial.println(nbrBadCRC);
    }
    attente(100);
    reponseNavigation = askNavigation();
	}
}

//----------------MISE A JOUR DU TEMPS DE MATCH----------------
void majTemps()
{
  tempsRestant = ( TEMPS_MATCH - (millis() - timeInit) ) / 1000;
  if ( tempsRestant <= 0 )
  {
    finMatch();
  }
}

//----------------PROCEDURE DE FIN DE MATCH----------------
void finMatch()
{
	// Stopper les moteurs
	sendNavigation(255, 0, 0);
	// Boucle infinie
	while(1)
	{
		// Stopper les moteurs
		sendNavigation(255, 0, 0);
    // Stoppe la Balise
    digitalWrite(pinBalise,LOW);
	}
}
