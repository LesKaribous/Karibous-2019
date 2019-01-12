#include <Arduino.h>
#include <Wire.h>
#include "stdlib.h"
#include "U8glib.h"
#include "image.cpp"
#include "DFRobotDFPlayerMini.h"

// Adresse I2C du module d'IHM'
#define ADRESSE 80
// Boutons
#define TEAM 0
#define STRATEGIE_1 1
#define STRATEGIE_2 2
#define ADVERSAIRE 3
// Couleur Equipe
#define BLEU 0
#define JAUNE 1
// Codes d'erreur
#define NO_ERROR 0
#define BUTTON_ERROR 1
#define COM_ERROR 2

#define INSERE_TIRETTE 0
#define ENLEVE_TIRETTE 1
#define DEBUT_MATCH 2
#define INIT 3


void receiveEvent(int HowMany);
void requestEvent();
void majButton();
void draw_init(void);
void draw_match(void);

const int tableBouton[16][5]=
{
	{0,0,0,0,1023},
	{0,0,0,1,269},
	{0,0,1,0,632},
	{0,0,1,1,216},
	{0,1,0,0,489},
	{0,1,0,1,196},
	{0,1,1,0,336},
	{0,1,1,1,165},
	{1,0,0,0,765},
	{1,0,0,1,229},
	{1,0,1,0,448},
	{1,0,1,1,189},
	{1,1,0,0,371},
	{1,1,0,1,173},
	{1,1,1,0,276},
	{1,1,1,1,149}
};
const int tableBouton2[4][5]=
{
	{0,0,0,0,1022},
	{0,0,0,1,266},
	{1,0,0,0,761},
	{1,0,0,1,227},
};

bool button[4];
const int pinButton = A9;
bool newCommand = false;
char error_com = NO_ERROR, error_button = NO_ERROR ;
byte command = 0 ;
byte boutton = 0 ;
char etat = INIT ;
int tempsMatch = 90000; // 90 secondes
int tempsMatchInit = 0;
char etat_match = 0 ;
int valueButton = 0 ;

U8GLIB_ST7920_128X64_1X u8g(12, 10, 11);
DFRobotDFPlayerMini myDFPlayer;

void setup()
{
	Serial1.begin(9600);
	Wire.begin(ADRESSE);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);
	delay(1000);

	if (!myDFPlayer.begin(Serial1))
	{  //Use softwareSerial to communicate with mp3.
		u8g.firstPage();
        do
		{
			u8g.setPrintPos(0, 40);
  		  	u8g.print("PROBLEME MODULE MP3");
        } while( u8g.nextPage() );
        delay(2000);
  	}
	myDFPlayer.volume(22);  //Set volume value. From 0 to 30
	myDFPlayer.play(1);  //Play the first mp3
	// Affichage du logo Les Karibous au demarrage
	u8g.firstPage();
    do {
      u8g.drawBitmapP( 0, 0, 16, 64, logoKaribous);
    } while( u8g.nextPage() );
    delay(2000);

}

void loop()
{
	majButton();
	u8g.firstPage();
    do {
		switch (etat)
		{
			case INIT:
			{
				draw_init();
				break;
			}
			case DEBUT_MATCH:
			{
				draw_match();
				break;
			}
		}
    } while( u8g.nextPage() );
    delay(100);

	if (etat == DEBUT_MATCH)
	{
		if (tempsMatch<=0)
		{
			tempsMatch = 0;
		}
		else
		{
			tempsMatch = 90000 - (millis()-tempsMatchInit);
		}

		if (tempsMatch <= 4000 && etat_match == 2)
		{
			myDFPlayer.play(3);
			etat_match = 3;
		}
		if (tempsMatch <=0 && etat_match == 3)
		{
			myDFPlayer.play(4);
			etat_match = 4;
		}
	}


}

void draw_init(void)
{
  u8g.setFont(u8g_font_04b_03);

  // Affichage de l'équipe
  u8g.setPrintPos(0, 10);
  u8g.print("Equipe ");
	if (button[TEAM]==BLEU) u8g.println("bleu");
	else u8g.println("jaune");

  // Affichage de la stratégie
  u8g.setPrintPos(0, 20);
  u8g.print("Strategie ");
	if (button[STRATEGIE_1]) u8g.print("1");
	else u8g.print("2");
  u8g.print(" avec option ");
    if (button[STRATEGIE_2]) u8g.println("1");
    else u8g.println("2");

	// Affichage de l'etat adverse
  u8g.setPrintPos(0, 30);
  	if (!button[ADVERSAIRE]) u8g.print("Detection adversaire activee");
  	else u8g.print("AUCUNE DETECTION ADVERSAIRE");

	// Affichage des erreurs
  u8g.setPrintPos(0, 40);
  	if (error_com) u8g.print("ERREUR DE COMMUNICATION");
  	else u8g.print("Aucune erreure");

	// Affichage de la valeur des bouttons
	u8g.setPrintPos(80, 40);
	u8g.print(valueButton);

	// Affichage début match
  u8g.setPrintPos(0, 50);
  	if (command == INSERE_TIRETTE) u8g.print("Inserer la tirette");
	else if (command == ENLEVE_TIRETTE) u8g.print("Enlever la tirette pour demarrer");
	else if (command == DEBUT_MATCH)
	{
		u8g.print("Debut du match");
		etat = DEBUT_MATCH;
		tempsMatchInit = millis();
		etat_match = 2;
	}

}

void draw_match(void)
{
	u8g.setFont(u8g_font_fub49n);
	u8g.setPrintPos(20, 55);
	u8g.print(tempsMatch/1000);
}

void majButton()
{
	// Les boutons sont reliés via un réseau de resistances sur une pin
	// analogique du microcontroleur.
	// La resistance de pullup initiale du circuit est de 4,7k
	// Les resistances suivantes sont : 1k / 3,3K / 4,7k
	// Les resistances sont montées en séries avec les interrupteurs
	// l'ensemble est monté en parrallele
	// Attention : resistances avec tolérances de 1%
	// L'alimentation du circuit se fait en 5V.

	int marge = 2;
	bool tempButton[4];
	// Stock les anciennes valeurs des bouttons
	for (int i=0;i<4;i++)
	{
		tempButton[i]=button[i];
	}

	for (int i=0;i<=20;i++)
	{
		valueButton = (valueButton + analogRead(pinButton))/2;
		delay(15);
	}

	for(int i=0;i<4;i++)
	{
		if ( valueButton <= (tableBouton2[i][4]+ 8) && valueButton >= (tableBouton2[i][4]- 8) )
		{
			button[0] = tableBouton2[i][0];
			button[1] = tableBouton2[i][1];
			button[2] = tableBouton2[i][2];
			button[3] = tableBouton2[i][3];
			i = 255 ;
			error_button = NO_ERROR;
		}
	}

	/*
	for(int i=0;i<16;i++)
	{
		if ( valueButton <= (tableBouton[i][4]+ 3) && valueButton >= (tableBouton[i][4]- 3) )
		{
			button[0] = tableBouton[i][0];
			button[1] = tableBouton[i][1];
			button[2] = tableBouton[i][2];
			button[3] = tableBouton[i][3];
			i = 255 ;
			error_button = NO_ERROR;
		}
	}
	*/
	// Compare si il y a eu des nouvelles valeurs
	for (int i=0;i<4;i++)
	{
		if (button[i] != tempButton[i])
		{
			myDFPlayer.play(2);
			i = 255;
		}
	}
	for (int i =0;i<4;i++)
	{
		bitWrite(boutton,i,button[i]);
	}


}

void receiveEvent(int howMany)
{
	if(howMany == 1)
	{
		// Si une commande est envoyée
		command = Wire.read();
	}
	else if (howMany == 2)
	{
		// Si une commande est envoyée
		byte test = Wire.read(); // Premiere commande non stockée

		if (Wire.read() == boutton)
		{
			error_com = NO_ERROR;
		}
		else
		{
			error_com = COM_ERROR;
		}
	}
	else
	{
		// Il y a un soucis dans la communication
		// Enregistrer l'erreure pour l'envoyer sur l'écran
		error_com = COM_ERROR;
	}
	newCommand = true;
}

void requestEvent()
{
	// Envoi l'état des bouttons
	Wire.write(boutton);
}
