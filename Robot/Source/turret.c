/**
 * \file turret.c
 * \brief Fichier contenant les fonctions de controle de le tourelle
 * \author Stanislas BERTRAND
 * \version 1.0
 * \date 30 avril 2011
 */ 
 
 /* ---------------------------- */
 /* -------- Librairies -------- */
 /* ---------------------------- */
 
#include "../Header/ifi_default.h"
#include "../Header/ifi_aliases.h"

#include "../Header/turret.h"
#include "../Header/orders.h"
#include "../Header/sonar.h"
#include "../Header/serial_ports.h"
#include "../Header/communication.h"
#include <stdio.h>
#include <stdlib.h>

 /* ------------------------------------ */
 /* -------- Variables globales -------- */
 /* ------------------------------------ */

/**
 * \var Order_Turret_Angle
 * \brief Consigne de gisement de la tourelle sonar
 * Variable globale declaree dans user_routine_fast.c
*/

enum etat{acquisition,deplacement,emmition,idle};


 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */

 /**
  * \fn void turret_Init (void)
  * \brief Initialisation de la tourelle
  * \return Void
  */
void turret_Init(void)
{
	SERVO_TURRET = 127; // la tourelle est orienté en face.
	
	SonarInit();
}	

 /**
  * \fn void turret_handle(void)
  * \breif Gestion de la tourelle dans user_routines_fast.c -> User_Autonomous_Code()
  */
void turret_handle(void)
{
	static enum etat turret_etat = acquisition;
	static signed char angle = 0;
	static signed char angle_inc = ENV_ANGLE_INC;
	static int wait = 10;
	static int nb_acq = 3;
	
	switch(turret_etat)
	{
		case acquisition :
			SonarPulse();
			
			if(! (ValidSonarDistance && SonarDistance>5) )
				nb_acq--;
			else
				printf((char *)"SonarDistance : %d\n\r",SonarDistance);
			
			if(ValidSonarDistance || nb_acq<=0)
				turret_etat = emmition;
				
			break;
		case emmition :
			
			if(ValidSonarDistance && SonarDistance>5)
				while(!ENV_Data_Transmit(SonarDistance,SERVO_TURRET));
			else
				while(!ENV_Data_Transmit(255,SERVO_TURRET));
			
			turret_etat = deplacement;
			break;
		case deplacement :
			angle = angle + angle_inc;
			
			if(angle < -ENV_FOV_HALF || angle > ENV_FOV_HALF )
			{
				angle = (angle<0)?-ENV_FOV_HALF:ENV_FOV_HALF;
				angle_inc = -angle_inc;
			}
			
			SERVO_TURRET = (unsigned char) (angle + ENV_FOV_HALF)*255/ENV_FOV;
			
			//printf("Servo_Turret : %d \n\r",SERVO_TURRET);
			
			turret_etat = idle;
			break;
		case idle :
		
			wait--;
			
			if(wait<=0)
			{
				wait = 10;	
				turret_etat = acquisition;
			}
			break;
		
	}	
	
}	