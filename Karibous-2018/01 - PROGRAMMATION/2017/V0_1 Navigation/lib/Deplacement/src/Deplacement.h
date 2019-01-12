#ifndef Deplacement_h
#define Deplacement_h

#define FULL_STEP 1
#define HALF_STEP 2
#define QUARTER_STEP 4
#define EIGHTH_STEP 8
#define SIXTEENTH_STEP 16
#define THIRTYTH_STEP 32

#define NB_STEPPER_TURN 200	//Nombre de pas pour un tour sur un moteur pas à pas classique. Peut être ajusté avec setNbStep(int nbStep)

class Deplacement
{
public:

	Deplacement(int dirRight,
		int dirLeft,
		int stepRight,
		int stepLeft,
		int mode,
		double entraxe,
		double wheelDiameter,
		int reduction);
	/**
	 * Fonction permettant de mettre à jour la position du robot en fonction de
	 * la vitesse, de l'acceleration et de la position demandée. A appeler le
	 * plus souvent possible et au moins une fois par pas.
	 * @return Vrai si la distance n'est pas encore atteinte. Faux si la distance est atteinte
	 */
	bool run();
	/**
	 * Définie la vitesse maximum de rotation du robot
	 * @param speed Vitesse de rotation en pas/s
	 */
	void setMaxSpeed(unsigned long speed);
	/**
	 * Définie la valeur de la pente d'acceleration et de déceleration des moteurs
	 * @param accel Acceleration en pas /s2
	 */
	void setAcceleration(unsigned long accel);
	/**
	 * Permet de faire bouger le robot selon la méthode "turn and go" : rotation puis déplacement
	 * @param rot  rotation en degrées
	 * @param dist distance en pas
	 */
	void turnGo(long angle, long distance);
	/**
	 * Permet de faire aller le robot en X,Y relatif avec une orientation
	 * @param X           Coordonnée X souhaitée
	 * @param Y           Coordonnée Y souhaitée
	 * @param orientation Orientation souhaitée
	 */
	void goTo(long X, long Y, long orientation);
	/**
	 * Définit le poucentage du profil de vitesse à vitesse maximum constante
	 * @param percentage Pourcentage du profil entre 0 et 100
	 */
	void setMaxPercentageProfil(char percentage);
	/**
	 * Fait tourner le robot autour du centre de l'essieu d'un nombre de pas.
	 * @param angle Angle en pas.
	 */
	void turn(long angle);
	/**
	 * Fait aller le robot en ligne droite de la distance définie en pas.
	 * @param distance distance en pas.
	 */
	void go(long distance);
	/**
	 * Met à jour le nombre de pas pour faire un tour de moteur.
	 * Depend du mode de pilotage.
	 * @param nbStep nombre de pas du moteur. En genreral 200.
	 */
	void setNbStep(int nbStep);
	/**
	 * Fonction remplacant run() dans le cas de mouvements utilsiants les fonctions
	 * turnGo() et goTo().
	 * @return Vrai si la distance n'est pas encore atteinte. Faux si la distance est atteinte
	 */
	bool runGoTo();
	/**
	 * Met en pause le mouvement et enregistre les parametres actuels afin de
	 * proposer une reprise du mouvement via resume() si necessaire.
	 */
	void pause();
	/**
	 * Re-démarre un mouvement mis en pause precedemment.
	 */
	void resume();

	bool readPause();
	//DEPRECIATE
	//void setProfil();


protected:

	/**
	 * Active les sorties pour le pilotage des drivers
	 */
	void enableOutputs();
	/**
	 * Réalise un step en envoyant une impulsion sur les pattes de "step" des
	 * drivers. Le temps de l'impulsion est définie par la variable _minPulseWidth
	 */
	void Step();
	/**
	 * convertit la distance en mm en pas moteur en fonction des parametres du robot
	 * @param distance distance en mm
	 */
	void distanceToStep(long distance);
	/**
	 * Applique les parametres de direction sur les drivers
	 */
	void setDirection();
	/**
	 * Calcul les nouvelles valeurs de temps pour la gestion des accelerations
	 * et des vitesses.
	 */
	void computeSpeedAccel();
	/**
	 * Restreint l'angle en radians entre -pi et pi
	 * @param  angle Angle à restreindre ( en radians )
	 * @return       angle restreint ( en radians )
	 */
	double restPmPi(double angle);



	//DEPRECIATE
	//void speedToTime();
	//void accelToTime();


private:
	/**
	 * Temps minimum d'une impulsion en microsecond
	 */
	unsigned int _minPulseWidth;

	/**
	 * Pins pour le pilotage des drivers
	 * 0 : dirRight  1 : dirLeft  2 : stepRight  3 : stepLeft
	 */
	int _pins[4];


	unsigned long _lastTime;
	/**
	 * Vitesse souhaitée maximum du robot en pas/s
	 */
	unsigned long _speed;
	/**
	 *  Acceleration souhaitée maximum du robot en pas/s2
	 */
	unsigned long _accel;
	/**
	 * Nombre de pas à réaliser
	 */
	unsigned long _targetStep;
	/**
	 * Nombre de pas à réaliser. Utilisé par les fonctions pause()
	 * et resume()
	 */
	unsigned long _targetStepTemp;
	/**
	 * Nombre de pas déja effectués
	 */
	unsigned long _currentStep;
	/**
	 * Nombre de pas déja effectués. Utilisé par les fonctions pause()
	 * et resume()
	 */
	unsigned long _currentStepTemp;
	/**
	 * Direction du moteur gauche
	 */
	bool _dirMotorLeft;
	/**
	 * Direction du moteur droit
	 */
	bool _dirMotorRight;
	/**
	 * Position en X actuelle du robot
	 */
	long _XActu;
	/**
	 * Position en Y actuelle du robot
	 */
	long _YActu;
	/**
	 * Orientation actuelle du robot
	 */
	long _orientationActu;
	/**
	 * Temps actuel entre deux pas
	 */
	unsigned long _currentStepTime;
	/**
	 * Distance en pas de la phase d'acceleration et de decceleration.
	 */
	unsigned long _accelDistance;
	/**
	 * Temps entre deux pas pour le pas initial
	 * Variable de calcul de l'equation 17
	 */
	double _P1;
	/**
	 * Constant multiplier
	 * Variable de calcul de l'equation 19
	 */
	double _R;
	/**
	 * Variable multiplicatrice
	 * Variable de calcul de l'equation 20
	 */
	double _m;
	/**
	 * Temps entre deux pas. A recalculer tous les pas pour gerer
	 * l'acceleration et la decceleration.
	 * Variable de calcul de l'equation 23
	 */
	double _Pa;
	/**
	 * Variable de calcul de l'equation 23
	 */
	double _q;

	/**
	 * Nombre de step par tour de moteur. Défini par le mode de pilotage et par
	 * le nombre max de step des moteurs pas à pas.
	 */
	int _nbStep;
	/**
	 * Mode de pilotage du Driver :
	 * FULL_STEP 1 - HALF_STEP 2 - QUARTER_STEP 4 -
	 * EIGHTH_STEP 8 - SIXTEENTH_STEP 16 - THIRTYTH_STEP 32
	 */
	int _mode;
	/**
	 * Valeur de l'entraxe du robot ( distance entre les roues ).
	 */
	double _entraxe;
	/**
	 * Diametre des roues.
	 */
	double _wheelDiameter;
	/**
	 * Valeur de l'étage de reduction.
	 */
	int _reduction;
	/**
	 * Perimetre des roues.
	 */
	double _wheelPerimeter;
	/**
	 * Resolution d'un pas en mm.
	 */
	double _stepResolution;
	/**
	 * Longueur de l'arc de rotation en mm.
	 */
	double _arcDistance;
	/**
	 * Nombre de pas à réaliser pour effectuer une distance.
	 * Utilisé pour goTo() et turnGo()
	 */
	long _targetDistStep;
	/**
	 * Nombre de pas à réaliser pour effectuer une rotation.
	 * Utilisé pour goTo() et turnGo()
	 */
	long _targetArcStep;
	/**
	 * Etat de l'action goTo() en cours
	 */
	char _stateGoTo;
	/**
	 * Etat de l'action run()
	 */
	bool _stateRun;
	/**
	 * Etat de la mise en pause
	 */
	bool _statePause;
	/**
	 * Distance demandée en mm
	 */
	double _distance;
	/**
	 *  Angle demandé en radians
	 */
	double _angle;

	//DEPRECIATE
	//unsigned long _percentages[3];
	//unsigned long _profileSteps[3];
	//unsigned long _incAccTime;
	//unsigned int _stepTime;
	//unsigned int _accelTime;
	//unsigned int _maxSpeedTime;

};
#endif
