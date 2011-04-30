/*
*	Include sonar.h in all file where any of these functions are used
*	1/ Put SonarInit() in User_Initialization() 
*		in the user_rouines.c
*	2/ Put SonarHandle() in InterruptHandlerLow()
*		in user_routines_fast.c
*	3/ Use SonarPulse() to get distance in SonarDistance
* 
*	Check on which interrupt and which digital output the sonar is in
*	Check digital output initialization
*/
#include "../Header/sonar.h"
#include "../Header/ifi_aliases.h"
#include "../Header/ifi_default.h"
#include "../Header/ifi_utilities.h"

unsigned int SonarDistance;
short ValidSonarDistance;

void SonarInit()
{
	// Edge falling interrupt 1 detection
	INT1_EDG = 0;
	INT1_PRY =0;
	// Timer 1 : 16bit , prescale 2, integrated clock
	T1CON = T1_16BIT + T1_PS2 + T1_INTCK;
	// timer 1 interrupt priority low
	T1_IP = 0; 
}
void SonarHandle()
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
void SonarPulse()
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
	    DIGOUT = 1 ;
		// Timer 1 ON
	    T1_ON = 1 ; 
		// wait for timer 1 overflow which count 20 us 
	    while(!T1_IF);
		//reset timer flag
	    T1_IF = 0 ;
		// stop sending pulse
	    DIGOUT = 0 ;
		//enable timer 1 interrupt
	    T1_IE = 1 ; 
	    // Enable interrupt 1
	    INT1_ENBL = 1; 
		// wait for pulse to come back (interupt 1) or timer overflow
		while(T1_ON);
	}
}