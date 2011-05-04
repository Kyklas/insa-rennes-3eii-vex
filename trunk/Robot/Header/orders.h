/**
 * \file orders.h
 * \brief Fichier contenant les fonctions de traitement des ordres recus / emis entre le robot et le PDA 
 * \author Kevin WYSOCKI
 * \version 1.0
 * \date 4 avril 2011
 */

#ifndef _ORDERS_H
#define _ORDERS_H

 /* ------------------------------------ */
 /* -------- Variables globales -------- */
 /* ------------------------------------ */

/**
 * \var Order_Turret
 * \brief Consigne de position de la tourelle sonar
 * 
*/
extern volatile unsigned char Order_Turret ;

/**
 * \var Order_Motor_Left
 * \brief Consigne de vitesse du moteur gauche
 * 
*/
extern volatile unsigned char Order_Motor_Left;

/**
 * \var Order_Motor_Right
 * \brief Consigne de vitesse du moteur droit
 * 
*/
extern volatile unsigned char Order_Motor_Right;

 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */

 /**
  * \fn void Order_Turret_Update (char Order_Turret)
  * \brief Modifie la consigne de gisement de la tourelle
  * \param[in,out] Order_Turret Consigne de gisement de la tourelle
  * \return Void
 */
void Order_Turret_Update (char Order_Turret);

 /**
  * \fn void Order_Motor_Left_Update (unsigned char Order_Motor)
  * \brief Modifie la consigne de vitesse du moteur gauche
  * \param[in,out] Order_Motor Consigne de vitesse du moteur gauche
  * \return Void
 */
void Order_Motor_Left_Update (unsigned char Order_Motor);

/**
  * \fn void Order_Motor_Right_Update (unsigned char Order_Motor)
  * \brief Modifie la consigne de vitesse du moteur droit
  * \param[in,out] Order_Motor Consigne de vitesse du moteur droit
  * \return Void
 */
void Order_Motor_Right_Update (unsigned char Order_Motor);

/**
  * \fn void CMD_Handler(void)
  * \brief Gestionnaire des commandes recus
  * \return Void
 */

void CMD_Handler(void);

/**
  * \fn char ENV_Data_Transmit(unsigned char Distance, char Angle)
  * \brief Méthode d'envoie des informations d'environement
  * \param[in] Distance Distance vu par le sonar
  * \param[in] Angle Angle de la tourelle
  * \return TRUE si l'envoie est effectif, FALSE sinon
 */

char ENV_Data_Transmit(unsigned char Distance, char Angle);

#endif