#include <Adafruit_NeoPixel.h>
#include <Servo.h>


// DECLARATION DE LA PARTIE GESTION DES LEDS

// Définition des PIN pour les 2 sorties LED
#define BANDEAU_PIN    6
#define K_PIN 7

// Nombre de LED par PIN
#define BANDEAU_COUNT 7
#define K_COUNT 7

#define CAPTEUR_PIN 14

// Déclaration des 2 objets NeoPixels
Adafruit_NeoPixel bandeau(BANDEAU_COUNT, BANDEAU_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel circle(K_COUNT, K_PIN, NEO_GRB + NEO_KHZ800);

// DECLARATION DE LA PARTIE GESTION DU SERVO
// Déclaration du Servomoteur
Servo myservo;


void setup() 
{

  pinMode(CAPTEUR_PIN,INPUT);
  
  // Initiatilisation des 2 objets NeoPixels:

  bandeau.begin();           // INITIALIZE du Bandeau
  bandeau.show();            // Turn OFF all pixels ASAP
  bandeau.setBrightness(150); // Set BRIGHTNESS to about 1/5 (max = 255)

  circle.begin();           // INITIALIZE du Cercle dans le K
  circle.show();            // Turn OFF all pixels ASAP
  circle.setBrightness(150); // Set BRIGHTNESS to about 1/5 (max = 255)

   

  delay(2000);
}

void loop() {

  while(digitalRead(CAPTEUR_PIN)==LOW)
  {
    delay(200);
  }
  
  // Demarrage servo
  // Initialisation du Servomoteur :
  myservo.attach(9); 
  myservo.write(140);
  // Code pour allumer les leds du bandeau progressivement toutes les secondes:
  for (int i = 6; i >= 0; i--) {
    bandeau.setPixelColor(i, 255);
    bandeau.show();
    delay(900);
  }

  // Code pour allumer les leds du cercle dans le K :
  for (int i = 6; i >= 0; i--) {
    circle.setPixelColor(i, 255);
    circle.show();
    delay(50);
    circle.clear();
    delay(100);
  }

  circle.clear();
  delay(200);

  //Fin du programme
  myservo.detach();
  while(1)
  {
    for (int j = 6; j >= 0; j--)
    {
      circle.setPixelColor(j, 0);
    }
    circle.show();
    delay(500);
    
    for (int j = 6; j >= 0; j--)
    {
      circle.setPixelColor(j, 255);
    }
    circle.show();
    delay(500);
  }

  //Fin du programme

}
