#include "Deplacement.h"

void setup()
{

	//Initialisation des I/O
	pinMode(pinReset1, OUTPUT);
	pinMode(pinSleep1, OUTPUT);
	pinMode(pinReset2, OUTPUT);
	pinMode(pinSleep2, OUTPUT);

	digitalWrite(pinReset1, HIGH);
	digitalWrite(pinSleep1, HIGH);
	digitalWrite(pinReset2, HIGH);
	digitalWrite(pinSleep2, HIGH);


	//Initialisation de la communication Serie, I2C, I2C Event
	Serial.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);


	//Initialisation Moteur
	MGauche.setMaxSpeed(VitesseMaxGauche);
	MGauche.setAcceleration(AccelMax);

	MDroit.setMaxSpeed(VitesseMaxDroite);
	MDroit.setAcceleration(AccelMax);

	/*
	pinMode(ADVERSAIRE_ARRIERE, INPUT_PULLUP);
	pinMode(ADVERSAIRE_AVANT, INPUT_PULLUP);

	for(int i = 0;i<4;i++)
	{
		pinMode(PIN_BORDURE[i], INPUT_PULLUP);
	}
	*/
}


void loop()
{
	MGauche.run();
	MDroit.run();

	updatePos();
	adversaire();
	goTo();
	bordure();

	if (fonction == 255) FIN_MATCH();

}


void updatePos()
{
	if (newPos==DISPONIBLE)
	{
		etatRotation = PREVU ;
		etatAvance = PREVU ;
		// Traitement de l'information
		NewX=relativeRequest[1]*FacteurX;
		NewRot=relativeRequest[0]*FacteurRot;
	}
	newPos = VALIDEE;
}

void goTo()
{
	if(absoluteRequest[1] != currenPos[1] || absoluteRequest[2] != currenPos[2] && !etatABS){
		etatABS = true;
		int Dx, Dy;
		Dx = absoluteRequest[1] - curentPos[1];
		Dy = absoluteRequest[2] - curentPos[0]

		relativeRequest[0] = atan2(Dy, Dx) - curentPos[0];
		relativeRequest[1] = sqrt((Dx*Dx) + (Dy*Dy));

		targetRot = absoluteRequest[0];

	}else if(etatABS){
		turnGo();
		if(etatAvance == FINI){
			relativeRequest[0] = targetRot;
			relativeRequest[1] = 0;
			etatABS = false;
			etatLastRot = true;
			currentPos = absoluteRequest;
		}
	}else{
		turnGo();
	}
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
	  if(etatLastRot) etatLastRot = false;
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


void receiveEvent(int howMany)
{

	if(howMany == 6)
	{
		// Si un déplacement relatif est demandé
		// On receptionne les données
		for (int i=0;i<=5;i++)
		{
			bufNavRelatif[i]=Wire.read();
		}

		// On calcul le CRC
		crcNavRelatif = CRC8.smbus(bufNavRelatif, sizeof(bufNavRelatif)-1); //On enleve le CRC
		//Serial.println(crcNavRelatif);
		// On regarde si le CRC calculé correspond à celui envoyé
		if (crcNavRelatif==bufNavRelatif[sizeof(bufNavRelatif)-1])
		{
			// CRC ok
			// On traite les données
			fonction = bufNavRelatif[0];
			relativeRequest[0]= bufNavRelatif[1] << 8 | bufNavRelatif[2];
			relativeRequest[1]= bufNavRelatif[3] << 8 | bufNavRelatif[4];
			optionAdversaire = bitRead(fonction, 0);
			optionRecalage = bitRead(fonction, 1);
			optionRalentit = bitRead(fonction,2);
			// On indique qu'une nouvelle position est disponible
			newPos = DISPONIBLE;
		}
		else
		{
			// CRC nok - la donnée est erronée
			// On indique que la prochaine position est erronée pour en renvois eventuel
			newPos = ERRONEE;
		}
	}

	if(howMany == 8)
	{
		// Si un déplacement relatif est demandé
		// On receptionne les données
		for (int i=0;i<=7;i++)
		{
			bufNavAbsolu[i]=Wire.read();
		}

		// On calcul le CRC
		crcNavAbsolu = CRC8.smbus(bufNavAbsolu, sizeof(bufNavAbsolu)-1); //On enleve le CRC
		//Serial.println(crcNavRelatif);
		// On regarde si le CRC calculé correspond à celui envoyé
		if (crcNavAbsolu==bufNavAbsolu[sizeof(bufNavAbsolu)-1])
		{
			// CRC ok
			// On traite les données
			fonction = bufNavAbsolu[0];
			absoluteRequest[0]= bufNavAbsolu[1] << 8 | bufNavAbsolu[2];
			absoluteRequest[1]= bufNavAbsolu[3] << 8 | bufNavAbsolu[4];
			absoluteRequest[2]= bufNavAbsolu[5] << 8 | bufNavAbsolu[6];
			optionAdversaire = bitRead(fonction, 0);
			optionRecalage = bitRead(fonction, 1);
			optionRalentit = bitRead(fonction,2);
			// On indique qu'une nouvelle position est disponible
			newPos = DISPONIBLE;
		}
		else
		{
			// CRC nok - la donnée est erronée
			// On indique que la prochaine position est erronée pour en renvois eventuel
			newPos = ERRONEE;
		}
	}
}


void requestEvent()
{

	if ( etatAvance == FINI && etatRotation == FINI && newPos == VALIDEE && !etatABS && !etatLastRot)
  {
    // Mouvement terminé
		Wire.write("O");
		//Serial.println('O');
	}
	else if (newPos == ERRONEE)
	{
    // Commande non validé
		Wire.write("E");
		//Serial.println('N');
	}
	else
	{
    // Mouvement non terminé
		Wire.write("N");
		//Serial.println('N');
	}
}
