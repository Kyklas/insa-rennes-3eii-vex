/**
 * \file turret.h
 * \brief Fichier contenant les fonctions de controle de le tourelle
 * \author Stanislas BERTRAND
 * \version 1.0
 * \date 30 avril 2011
 */

#ifndef _TURRET_H
#define _TURRET_H

 /* --------------------------- */
 /* --------- Macros ---------- */
 /* --------------------------- */
 
 /**
  * \def SERVO_TURRET
  * PWM link to the servo of the turret
  */
#define SERVO_TURRET	pwm03

 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */
 /**
  * \fn void turret_Init (void)
  * \brief Initialisation de la tourelle
  * \return Void
  */
void turret_Init(void);

 /**
  * \fn void turret_handle(void)
  * \breif Gestion de la tourelle dans user_routines_fast.c -> User_Autonomous_Code()
  */
void turret_handle(void);

#endif