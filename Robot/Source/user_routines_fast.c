/**
 * \file user_routines_fast.c
 * \brief Fonctions executes par le main et fonctions d'interruptions - Fournis par Vex 
 * \author Vex Robotics
 */ 

/*******************************************************************************
* FILE NAME: user_routines_fast.c <VEX VERSION>
*
* DESCRIPTION:
*  This file is where the user can add their custom code within the framework
*  of the routines below. 
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
* OPTIONS:  Interrupts are disabled and not used by default.
*
*******************************************************************************/

#include "../Header/ifi_aliases.h"
#include "../Header/ifi_default.h"
#include "../Header/ifi_utilities.h"
#include "../Header/user_routines.h"
#include "../Header/ifi_picdefs.h"
#include "../Header/serial_ports.h"
#include "../Header/orders.h"
#include "../Header/communication.h"
#include "../Header/delays.h"
#include "../Header/sonar.h"
#include "../Header/turret.h"
#include <stdio.h>
#include <stdlib.h>

/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/


#pragma code InterruptVectorLow = LOW_INT_VECTOR
void InterruptVectorLow (void)
{
  _asm
    goto InterruptHandlerLow  /*jump to interrupt routine*/
  _endasm
}


#pragma code
#pragma interruptlow InterruptHandlerLow save=PROD /* You may want to save additional symbols. */

/**
 * \fn void InterruptHandlerLow () 
 * \brief Procedure d'interruption sur les vecteurs de priorite basse
 * 		  Contient la gestion des commandes recues - Interruption sur le port serie 2
*/
void InterruptHandlerLow ()     
{                        
	unsigned char int_byte;      
      if (PIR1bits.RC1IF && PIE1bits.RC1IE) // rx1 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_ONE_RX
		Rx_1_Int_Handler(); // call the rx1 interrupt handler (in serial_ports.c)
		#endif
	}                              
	else if (PIR1bits.TX1IF && PIE1bits.TX1IE) // tx1 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_ONE_TX
		Tx_1_Int_Handler(); // call the tx1 interrupt handler (in serial_ports.c)
		#endif
	}
	else if (PIR3bits.RC2IF && PIE3bits.RC2IE) // rx2 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_TWO_RX
		Rx_2_Int_Handler(); // call the rx2 interrupt handler (in serial_ports.c)
		#endif		
	}                              
	else if (PIR3bits.TX2IF && PIE3bits.TX2IE) // tx2 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_TWO_TX
		Tx_2_Int_Handler(); // call the tx2 interrupt handler (in serial_ports.c)
		#endif
	}

	SonarHandle();
     
  if (INTCON3bits.INT2IF && INTCON3bits.INT2IE)       /* The INT2 pin is RB2/DIG I/O 1. */
  { 
    INTCON3bits.INT2IF = 0;
  }
  else if (INTCON3bits.INT3IF && INTCON3bits.INT3IE)  /* The INT3 pin is RB3/DIG I/O 2. */
  {
    INTCON3bits.INT3IF = 0;
  }
  else if (INTCONbits.RBIF && INTCONbits.RBIE)  /* DIG I/O 3-6 (RB4, RB5, RB6, or RB7) changed. */
  {
    int_byte = PORTB;          /* You must read or write to PORTB */
    INTCONbits.RBIF = 0;     /*     and clear the interrupt flag         */
  }                                        /*     to clear the interrupt condition.  */
}


/**
 * \fn void User_Autonomous_Code(void)
 * \brief Code utilisateur en mode autonome - Boucle executee toute les 18.5ms
		  Contient le code de deplacement et d'acquisition entre les fonctions Getdata() et Putdata()
*/
void User_Autonomous_Code(void)
{
  /* Initialize all PWMs and Relays when entering Autonomous mode, or else it
     will be stuck with the last values mapped from the joysticks.  Remember, 
     even when Disabled it is reading inputs from the Operator Interface. 
  */
  pwm01 = pwm02 = pwm03 = pwm04 = 127;
  pwm05 = pwm06 = pwm07 = pwm08 = 127;
  pwm09 = pwm10 = pwm11 = pwm12 = 127;

  while (autonomous_mode)   /* DO NOT CHANGE! */
  {
    if (statusflag.NEW_SPI_DATA)      /* 18.5ms loop area */
    {
      	Getdata(&rxdata);   /* DO NOT DELETE, or you will be stuck here forever! */

		
		if (!rc_dig_in01)
		{
			pwm01 = 127;
			pwm02 = 127;
			pwm03 = 127;
			Order_Motor_Left_Update (127);
			Order_Motor_Right_Update (127);
		}
		else
		{
			/* Ajuster la valeurs des signaux PWM de commande des moteurs */
			pwm01 = Limit_Mix(2000 + Order_Motor_Left + Order_Motor_Right - 127);
			pwm02 = Limit_Mix(2000 + Order_Motor_Left - Order_Motor_Right + 127);
			
			turret_handle();	
		}
			
      	Putdata(&txdata);   /* DO NOT DELETE, or you will get no PWM outputs! */
    }
    Process_Data_From_Local_IO();
  }
}



void Process_Data_From_Local_IO(void)
{
      /* This code is executed at every loop, this handle the incomming communication */
  	if(rc_dig_in01)
  	{
	  	CMD_Handler();
	}
	else
	{/* bluetooth communication is off */
		while (Serial_Port_Two_Byte_Count())
			Read_Serial_Port_Two();
			
		if(PIE3bits.TX2IE)
			printf((char *)"Sending data ... !!\n\r");
	}
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
