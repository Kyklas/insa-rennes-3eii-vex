/**
 * \file orders.h
 * \brief Fichier contenant les fonctions de traitement des ordres recus / emis entre le robot et le PDA 
 * \author Kevin WYSOCKI
 * \version 1.0
 * \date 4 avril 2011
 */

 #ifndef _ORDERS_H
#define _ORDERS_H

 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */
 /**
  * \fn void Cmd_Ack (unsigned char Cmd)
  * \brief Envoi un acquittement sur la commande recue au PDA
  * \param[in] Cmd Dernier ordre recu
  * \return Void
 */
void Cmd_Ack (unsigned char Cmd);

 /**
  * \fn void Order_Turret_Angle_Update (char Order_Turret)
  * \brief Modifie la consigne de gisement de la tourelle
  * \param[in,out] Order_Turret Consigne de gisement de la tourelle
  * \return Void
 */
void Order_Turret_Angle_Update (char Order_Turret);

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
 * \fn unsigned char Cmd_Receive (void)
 * \brief Procedure de reception d'une commande envoyee par le PDA
 * \param Void
 * \return La commande sur un octet
*/
unsigned char Cmd_Receive (void);

/**
 * \fn unsigned char Order_ENV_Transmit (unsigned char Distance, char Angle)
 * \brief Procedure d'envoi des infos d'environnement au PDA
 * \param[in] Distance Distance de l'objet le plus proche pour un angle donne
 * \param[in] Angle Angle de gisement de la tourelle
 * \return TRUE si l'acquittement du PDA est OK, FALSE sinon
*/
unsigned char Order_ENV_Transmit (unsigned char Distance, char Angle);


#endif