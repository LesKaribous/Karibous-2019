#include "Strategie.h"

void setup()
{
  pinMode(pinTirette,INPUT_PULLUP);
  pinMode(pinDetection,INPUT_PULLUP);
  pinMode(pinStrategie,INPUT_PULLUP);

  Serial.begin(9600);     //Demarrage d'un liaison série pour le debug
  Wire.begin();           //Demarrage de la liaison I2C

  u8g2.begin();           //Init du LCD
  u8g2_splash_screen();   //Affichage du Logo des Karibous
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
}

void loop()
{
  testDeplacement();
}

//----------------GESTION DES BOUTTONS DE L'IHM----------------
void bouttonIHM()
{
  detection = digitalRead(pinDetection) ;
  strategie = digitalRead(pinStrategie) ;
  tirette   = digitalRead(pinTirette)   ;

  if(analogRead(pinEquipe)>10) equipe = true ;
  else equipe = false ;

  if(analogRead(pinCheck)>10) check = true ;
  else check = false ;

  if(!check) initRobot();
}

//----------------STRATEGIES----------------
void testDeplacement()
{
  turnGo(non,0,false,45,600);
}

//----------------PROCEDURE D'ATTENTE----------------
void attente(int temps)
{
	int initTemps = millis();
	while( (millis()-initTemps) <= temps)
	{
		majTemps();
		//u8g2_menu_pendant_match();
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
  u8g2.clearBuffer();
  u8g2_prepare();
    u8g2.setFont(u8g2_font_4x6_tf);
    // Etat equipe :
      u8g2.drawStr( 0, 10, "Equipe :");
      if ( equipe == jaune ) u8g2.drawStr( 40, 10, "JAUNE");
      else u8g2.drawStr( 40, 10, "VIOLET");
    // Etat detection:
      u8g2.drawStr( 0, 20, "Detection :");
      if ( detection ) u8g2.drawStr( 50, 20, "OUI");
      else u8g2.drawStr( 50, 20, "NON ATTENTION");
    // Etat strategie :
      u8g2.drawStr( 0, 30, "Strategie :");
      if ( strategie ) u8g2.drawStr( 40, 30, "PRIMAIRE");
      else u8g2.drawStr( 40, 30, "SECONDAIRE");
    // Etat tirette :
      u8g2.drawStr( 0, 40, "Etat tirette :");
      if ( tirette ) u8g2.drawStr( 60, 40, "Attente tirette");
      else u8g2.drawStr( 60, 40, "Tirette OK");
  u8g2.sendBuffer();
}

//----------------INITIALISATION DU ROBOT----------------
void initRobot()
{
  //-------Affichage Init-------
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.drawStr( 10, 2, "Init");
  u8g2.sendBuffer();
  delay(1000);
  //-------Recalage bordure-------
  // turnGo(non,0,true,-90,-370);
  // delay(500);
  // turnGo(non,0,true,0,320);
  // delay(500);
  // turnGo(non,0,true,90,-110);
  // delay(500);
  // turnGo(non,0,true,0,70);
  // delay(500);
  //-------Checklist-------
  // A FINIR !
  u8g2.setFont(u8g2_font_fub14_tf);
  for(int i=0;i<6;i++)
  {
    u8g2.clearBuffer();
    u8g2.drawStr( 10, 2, list[i]);
    u8g2.sendBuffer();
    delay(500);
    while(analogRead(pinCheck)>10){}  //Attente d'appui sur le bouton check
  }
}

//----------------DEMANDE L'ETAT DU DEPLACEMENT----------------
int askNavigation()
{
  int etatNavigation ;
  char reponseNavigation ;
  Wire.requestFrom(carteDeplacement, 1);
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
	Wire.beginTransmission(carteDeplacement);
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
	if ( equipe == jaune ) rot = -rot ;
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
	Wire.beginTransmission(carteDeplacement);
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
  tempsRestant = ( tempsMatch - (millis() - timeInit) ) / 1000;
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
	}
}
