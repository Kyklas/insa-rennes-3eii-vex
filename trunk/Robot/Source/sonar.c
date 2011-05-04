/**
 * \file sonar.c
 * \brief Implementation du driver pour le sonar
 * \author Stanislas BERTRAND
 * \version 1.0
 * \date 4 avril 2010
 */ 
#include "../Header/sonar.h"
#include "../Header/ifi_aliases.h"
#include "../Header/ifi_default.h"
#include "../Header/ifi_utilities.h"


/**
 * \var SonarDistance
 * \brief Distance mesuré par le sonar
 * 
*/
unsigned int SonarDistance;

/**
 * \var ValidSonarDistance
 * \brief information concernant SonarDistance, TRUE la distance est correct, FALSE incorrect
 * 
*/
short ValidSonarDistance;

/**
 * \fn void SonarInit(void)
 * \brief Methode d'initialisation du sonar
 * 
 * Initialise le Sonar
*/

void SonarInit(void)
{
	SONAR_IO_OUT = OUTPUT; // define the IO pin as an OUPUT
	SONAR_PULSE_OUT = 0;   // NO sonar emition
	
	// Edge falling interrupt 1 detection
	INT1_EDG = 0;
	INT1_PRY =0;
	// Timer 1 : 16bit , prescale 2, integrated clock
	T1CON = T1_16BIT + T1_PS2 + T1_INTCK;
	// timer 1 interrupt priority low
	T1_IP = 0; 
}

/**
 * \fn void SonarHandle(void)
 * \brief Gestionnaire du sonar
 *
 * Gere le timer et l'interrupt liée au Sonar
*/

void SonarHandle(void)
{
	if (  T1_IF && T1_IE )
	{
	  T1_IF = 0; // reset the flag
	  T1_ON = 0 ; // timer1 off
	  INT1_ENBL = 0; // interrupt 1 off
	  ValidSonarDistance = 0;
	  SonarDistance = -1;
	}
	// Sonar pusle receved
	if (INT1_FLG && INT1_ENBL)      
	{ 
	T1_ON = 0 ; // timer 1 off
	INT1_FLG = 0; // reset the flag

	SonarDistance = TMR1L;
	SonarDistance += TMR1H*256;
	// the time is per 200 ns and sound goes at 0.007cm/200ns
	// the time is twice the distance so 0.0035
	SonarDistance = SonarDistance * 0.0035;
	ValidSonarDistance = 1;
	}
	
}

/**
 * \fn void SonarPulse(void)
 * \brief Methode effectuant une mesure
 *
 * Met à jour les variables globale > SonarDistance < et > ValidSonarDistance <
*/

void SonarPulse(void)
{
	if(! T1_ON )
    {
	    ValidSonarDistance = 0;
		// Timer 1 interrupt off
		T1_IE = 0 ; 
	    // Put 64285 in timer to count 1250 x 200ns = 250us
		// 250 us is 10 period at 40KHz
	    // before timer over flow
		TMR1H = 0xFB;
	    TMR1L = 0x1D;
	    // Send pulse
	    SONAR_PULSE_OUT = 1 ;
		// Timer 1 ON
	    T1_ON = 1 ; 
		// wait for timer 1 overflow which count 20 us 
	    while(!T1_IF);
		//reset timer flag
	    T1_IF = 0 ;
		// stop sending pulse
	    SONAR_PULSE_OUT = 0 ;
		//enable timer 1 interrupt
	    T1_IE = 1 ; 
	    // Enable interrupt 1
	    INT1_ENBL = 1; 
		// wait for pulse to come back (interupt 1) or timer overflow
		while(T1_ON);
	}
}