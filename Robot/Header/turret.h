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

void turret_Init(void);
void turret_handle(void);

#endif