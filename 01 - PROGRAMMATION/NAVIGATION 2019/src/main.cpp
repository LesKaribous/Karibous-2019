#include "Navigation.h"

void setup()
{
	//------Déclaration des I/O------
	pinMode(pinSleep, OUTPUT);

  pinMode(pinM0, OUTPUT);
  pinMode(pinM1, OUTPUT);
  pinMode(pinM2, OUTPUT);

  pinMode(adversaireAvant, INPUT_PULLUP)	;
  pinMode(adversaireArriere, INPUT_PULLUP);

	pinMode(pinRobot,INPUT_PULLUP);

	for (int i=0;i<4;i++)
	{
		pinMode(PIN_BORDURE[i], INPUT_PULLUP)	;
	}

  //------Initilisation des I/O------
  // Passage en mode 1/4 de pas
  digitalWrite(pinM0, LOW);
  digitalWrite(pinM1, HIGH);
  digitalWrite(pinM2, LOW);
  // Pause des drivers avant le premier mouvement
  digitalWrite(pinSleep, LOW);
	// Init du type de robot
	typeRobot = digitalRead(pinRobot);
	changeTypeRobot(typeRobot);

	//------Initialisation des communications------
	Serial.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

	//------Initialisation Moteur------
	MGauche.setMaxSpeed(VitesseMaxGauche);
	MGauche.setAcceleration(AccelMaxGauche);

	MDroit.setMaxSpeed(VitesseMaxDroite);
	MDroit.setAcceleration(AccelMaxDroite);
}

void loop()
{
  //------Actualisation des moteurs------
	MDroit.run();
	MGauche.run();


  //------Affichage du debug------
  //debug();

  //------Maj focntions moteurs------
	updatePos();
	adversaire();
	goTo();
	bordure();

	if (fonction == 255) FIN_MATCH();
}

void changeTypeRobot(bool type)
{
	if(type == ROBOT_PRIMAIRE)
	{
		FacteurX					= primaireFacteurX ;
		FacteurDroit 			= primaireFacteurDroit ;
		FacteurGauche 		= primaireFacteurGauche ;
		FacteurRot 				= primaireFacteurRot ;

		VitesseMaxDroite 	= primaireVitesseMaxDroite ;
		VitesseMaxGauche 	= primaireVitesseMaxGauche ;
		VitesseMinDroite 	= primaireVitesseMinDroite ;
		VitesseMinGauche 	= primaireVitesseMinGauche ;
		AccelRot 					= primaireAccelRot ;
		AccelMinDroite 		= primaireAccelMinDroite ;
		AccelMinGauche 		= primaireAccelMinGauche ;
		AccelMaxDroite 		= primaireAccelMaxDroite ;
		AccelMaxGauche 		= primaireAccelMaxGauche ;
		AccelStop 				= primaireAccelStop ;

	} else {

		FacteurX					= secondaireFacteurX ;
		FacteurDroit 			= secondaireFacteurDroit ;
		FacteurGauche 		= secondaireFacteurGauche ;
		FacteurRot 				= secondaireFacteurRot ;

		VitesseMaxDroite 	= secondaireVitesseMaxDroite ;
		VitesseMaxGauche 	= secondaireVitesseMaxGauche ;
		VitesseMinDroite 	= secondaireVitesseMinDroite ;
		VitesseMinGauche 	= secondaireVitesseMinGauche ;
		AccelRot 					= secondaireAccelRot ;
		AccelMinDroite 		= secondaireAccelMinDroite ;
		AccelMinGauche 		= secondaireAccelMinGauche ;
		AccelMaxDroite 		= secondaireAccelMaxDroite ;
		AccelMaxGauche 		= secondaireAccelMaxGauche ;
		AccelStop 				= secondaireAccelStop ;
	}
}

void debug()
{
  Serial.print("newPos:");
  Serial.println(newPos);
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
		// Activer les moteurs
		digitalWrite(pinSleep, HIGH);
		newPos = VALIDEE;
	}
}

void goTo()
{
	if(absoluteRequest[1] != currentPos[1] || absoluteRequest[2] != currentPos[2] && !etatABS){
		etatABS = true;
		int Dx, Dy;
		Dx = absoluteRequest[1] - currentPos[1];
		Dy = absoluteRequest[2] - currentPos[0];

		relativeRequest[0] = atan2(Dy, Dx) - currentPos[0];
		relativeRequest[1] = sqrt((Dx*Dx) + (Dy*Dy));

		targetRot = absoluteRequest[0];

	}else if(etatABS){
		turnGo();
		if(etatAvance == FINI){
			relativeRequest[0] = targetRot;
			relativeRequest[1] = 0;
			etatABS = false;
			etatLastRot = true;
			currentPos[0] = absoluteRequest[0];
			currentPos[1] = absoluteRequest[1];
			currentPos[2] = absoluteRequest[2];
		}
	}else{
		turnGo();
	}
}

void turnGo()
{
  if ((presenceAvant && NewX>=0 && etatAvance == EN_COURS) || (presenceArriere && NewX<0 && etatAvance == EN_COURS) )
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
       if ((presenceAvant &&  NewX>=0 ) || (presenceArriere && NewX<0))
       {
          i=0;              //RAZ de l'iteration si toujours un obstacle
       }
       //delayMicroseconds(10);
       MGauche.run();
       MDroit.run();
     }

     MGauche.setAcceleration(AccelMaxGauche);
     MDroit.setAcceleration(AccelMaxDroite);

     MGauche.move(TempGauche);
     MDroit.move(TempDroit);
  }
  else
  {
    if (etatRotation == PREVU)
    {
      MGauche.setAcceleration(AccelRot);
      MDroit.setAcceleration(AccelRot);
			if (optionRalentit)
  		{
  			MGauche.setMaxSpeed(VitesseMinGauche);
  	        MDroit.setMaxSpeed(VitesseMinDroite);
  		}
  		else
  		{
  			MGauche.setMaxSpeed(VitesseMaxGauche);
  	        MDroit.setMaxSpeed(VitesseMaxDroite);
  		}
      MDroit.move(NewRot);
      MGauche.move(NewRot);
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
  	        MGauche.setAcceleration(AccelMinGauche);
  	        MDroit.setAcceleration(AccelMinDroite);
  		}
  		else
  		{
  			MGauche.setMaxSpeed(VitesseMaxGauche);
  	        MDroit.setMaxSpeed(VitesseMaxDroite);
  	        MGauche.setAcceleration(AccelMaxGauche);
  	        MDroit.setAcceleration(AccelMaxDroite);
  		}

        MDroit.move(-(NewX*FacteurDroit));
        MGauche.move(NewX*FacteurGauche);
        etatAvance = EN_COURS ;
    }
    if (etatAvance == EN_COURS && optionRecalage)
    {
      MGauche.setMaxSpeed(VitesseMinGauche);
      //MGauche.setAcceleration(AccelMax);
      MDroit.setMaxSpeed(VitesseMinDroite);
      //MDroit.setAcceleration(AccelMax);
	  MGauche.setAcceleration(AccelMinGauche);
	  MDroit.setAcceleration(AccelMinDroite);
      // Si on est à la fin du mouvement
      recalage();
    }
    if (MDroit.distanceToGo() == 0 && MGauche.distanceToGo() == 0 && etatAvance == EN_COURS)
    {
      etatAvance = FINI ;
			digitalWrite(pinSleep, LOW);
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
    MDroit.setAcceleration(AccelMaxDroite);
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
    MGauche.setAcceleration(AccelMaxGauche);
  }
}

void bordure()
{
	 for(int i = 0;i<4;i++)
	 {
	 	bitWrite(BORDURE,i,digitalRead(PIN_BORDURE[i]));
	 }
	 //Serial.println(BORDURE,BIN);
	 //delay(200);

}

// bool detection(int capteur, int iteration)
// {
// 	int valCapteur = 0;
// 	valCapteur = analogRead(capteur);
// 	for ( int i = 0;i<=iteration;i++)
// 	{
// 		valCapteur += analogRead(capteur);
// 	}
// 	valCapteur = valCapteur/iteration;
// 	return (valCapteur>=seuilAvant) ;
// }

void adversaire()
{
	// //Si la detection adverse est activée
	// if (!optionAdversaire)
	// {
	// 	// -----------Si on detecte un adversaire à l'avant-------------
	// 	if (digitalReadFast(adversaireAvant))
	// 	{
	// 		// Si on avait rien detecté pour le moment
	// 		if (!presenceAvantTemp)
	// 		{
	// 			presenceAvantTemp = true ;
	// 			angleAvant = analogRead(angleBalise);
	// 		}
	// 	}
	// 	else if (presenceAvantTemp)
	// 	{
	// 		angleAvant = analogRead(angleBalise)-angleAvant;
	// 		angleAvant = abs(angleAvant); // Voir doc Arduino. Ne rien mettre d'autre dans la fonction abs()
	// 		presenceAvantTemp = false ;
	// 		Serial.println(angleAvant);
	// 		if (angleAvant >= seuilAvant)
	// 		{
	// 			// Un adversaire est trop proche
	// 			presenceAvant = true ;
	// 			// On lance un timer d'attente que l'adversaire soit parti
	// 			avantTimeInit = millis();
	// 		}
	// 	}
	// 	// ------------Si on detecte un adversaire à l'arriere-------------
	// 	if (digitalReadFast(adversaireArriere))
	// 	{
	// 		// Si on avait rien detecté pour le moment
	// 		if (!presenceArriereTemp)
	// 		{
	// 			presenceArriereTemp = true ;
	// 			angleArriere = analogRead(angleBalise);
	// 		}
	// 	}
	// 	else if (presenceArriereTemp)
	// 	{
	// 		angleArriere = analogRead(angleBalise)-angleArriere;
	// 		angleArriere = abs(angleArriere); // Voir doc Arduino. Ne rien mettre d'autre dans la fonction abs()
	// 		presenceArriereTemp = false ;
	// 		//Serial.println(angleArriere);
	// 		if (angleArriere >= seuilArriere)
	// 		{
	// 			// Un adversaire est trop proche
	// 			presenceArriere = true ;
	// 			// On lance un timer d'attente que l'adversaire soit parti
	// 			arriereTimeInit = millis();
	// 		}
	// 	}
	// 	if((millis()-avantTimeInit)>=sensorTime && presenceAvant==true)
	// 	{
	// 		// L'adversaire est trop loin depuis sufisamment longtemps.
	// 		presenceAvant = false;
	// 		presenceAvantTemp = false ;
	// 	}
	// 	if((millis()-arriereTimeInit)>=sensorTime && presenceArriere==true)
	// 	{
	// 		// L'adversaire est trop loin depuis sufisamment longtemps.
	// 		presenceArriere = false;
	// 		presenceArriereTemp = false ;
	// 	}
	// }
	// else
	// {
	// 	presenceArriere = false;
	// 	presenceAvant = false;
	// }
	// Si la detection adverse est activée
	if (!optionAdversaire)
	{
		// Adversaire Avant
		if (digitalReadFast(adversaireAvant))
		{
			if (!presenceAvant)
			{
				presenceAvant = true ;
			}
			avantTimeInit = millis();
		}
		else
		{
			if((millis()-avantTimeInit)>=sensorTime)
			{
				presenceAvant = false;
			}
		}
		// Adversaire Arriere
		if (digitalReadFast(adversaireArriere))
		{
			if (!presenceArriere)
			{
				presenceArriere = true ;
			}
			arriereTimeInit = millis();
		}
		else
		{
			if((millis()-arriereTimeInit)>=sensorTime)
			{
				presenceArriere = false;
			}
		}
	}
	else
	{
		presenceArriere = false;
		presenceAvant = false;
	}

	// UNIQUEMENT EN DEBUG !!!!!!!!!!!
	/*
	if (presenceAvant || presenceArriere)
	{
		Serial.print(presenceAvant);
		Serial.print(" - ");
		Serial.println(presenceArriere);
		delay(200);
	}
	*/

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
			digitalWrite(pinSleep, LOW);
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
		Serial.println(crcNavRelatif);
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
		Wire.write('O');
		//Serial.println('O');
	}
	else if (newPos == ERRONEE)
	{
    // Commande non validé
		Wire.write('E');
		//Serial.println('N');
	}
	else
	{
    // Mouvement non terminé
		Wire.write('N');
		//Serial.println('N');
	}
}
