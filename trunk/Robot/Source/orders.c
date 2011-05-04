/**
 * \file orders.c
 * \brief Fichier contenant les fonctions de traitement des ordres recus / emis entre le robot et le PDA 
 * \author Kevin WYSOCKI
 * \version 1.0
 * \date 4 avril 2011
 */ 
 
 /* ---------------------------- */
 /* -------- Librairies -------- */
 /* ---------------------------- */

#include "../Header/ifi_default.h"
#include "../Header/ifi_aliases.h" 

#include "../Header/orders.h"
#include "../Header/serial_ports.h"
#include "../Header/communication.h"

#include <stdio.h>

 /* ------------------------------------ */
 /* -------- Variables globales -------- */
 /* ------------------------------------ */

/**
 * \var Order_Turret
 * \brief Consigne de position de la tourelle sonar
 * 
*/
volatile unsigned char Order_Turret ;

/**
 * \var Order_Motor_Left
 * \brief Consigne de vitesse du moteur gauche
 * 
*/
volatile unsigned char Order_Motor_Left = 0x00;

/**
 * \var Order_Motor_Right
 * \brief Consigne de vitesse du moteur droit
 * 
*/
volatile unsigned char Order_Motor_Right = 0x00;

/**
 * \var Order_ACK_Pending
 * \brief Information du la communication
 * 
 * 	TRUE - une commande est en cours d'envoie 
 *
 * 	FALSE - la communication est libre
*/

volatile unsigned char Order_ACK_Pending = FALSE;



 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */



/**
 * \fn void Order_Turret_Update (char Order_Turret)
 * \brief Modifie la consigne de gisement de la tourelle
 * \param[in,out] Order_Turret Consigne de gisement de la tourelle
 * \return Void
*/
void Order_Turret_Update (char Order)
{
	/* Met à jour la consigne de gisement de la tourelle */
	Order_Turret = Order;  
}

/**
 * \fn void Order_Motor_Left_Update (unsigned char Order_Motor)
 * \brief Modifie la consigne de vitesse du moteur gauche
 * \param[in,out] Order_Motor Consigne de vitesse du moteur gauche
 * \return Void
*/
void Order_Motor_Left_Update (unsigned char Order_Motor)
{
	/* Met à jour la consigne de vitesse du moteur gauche */
	Order_Motor_Left = Order_Motor ; 
}

/**
 * \fn void Order_Motor_Right_Update (unsigned char Order_Motor)
 * \brief Modifie la consigne de vitesse du moteur droit
 * \param[in,out] Order_Motor Consigne de vitesse du moteur droit
 * \return Void
*/
void Order_Motor_Right_Update (unsigned char Order_Motor)
{
	/* Met à jour la consigne de vitesse du moteur droit */
	Order_Motor_Right = Order_Motor ; 
}

/**
  * \fn char Wait_SP2_Byte_Count(unsigned short count)
  * \brief Méthode d'attente d'un nombre minimum de donnée sur le port série 2
  * prend en compte le master et l'état de la connection
  * \param[in] count nombre minimum de donnée attendu
  * \return TRUE si la connection est toujours active, FALSE sinon
 */

char Wait_SP2_Byte_Count(unsigned short count)
{
	while(Serial_Port_Two_Byte_Count() < count && rc_dig_in01)
	{
		Getdata(&rxdata);
		Putdata(&txdata);
	}
	
	if(rc_dig_in01)
		return TRUE;
	else
		return FALSE;
}	

/**
  * \fn void CMD_DPL_Handler(unsigned char cmd_ack)
  * \brief Gestionnaire de la commande CMD_DPL
  * \param[in] cmd_ack commande reçu
  * \return Void
 */

void CMD_DPL_Handler(unsigned char cmd_ack)
{	
	unsigned char byte1,byte2; 
	if(!Wait_SP2_Byte_Count(CMD_DPL_LEN))
		return; // we lost connection, quit handler
	
	/* Les deux octets sont reçus, on peut les lire */
	byte1 = Read_Serial_Port_Two();
	byte2 = Read_Serial_Port_Two();
	
	/* Mise à jour des consignes moteurs */
	Order_Motor_Left_Update (byte1);
	Order_Motor_Right_Update (byte2);
	
	/* Sending acknowledege to the command */
	Write_Serial_Port_Two (CMD_DPL_ACK); 
	
}

/**
  * \fn void CMD_ENV_ACK_Handler(unsigned char cmd_ack)
  * \brief Gestionnaire de la commande CMD_ENV_ACK
  * \param[in] cmd_ack commande reçu
  * \return Void
 */

void CMD_ENV_ACK_Handler(unsigned char cmd_ack)
{
	/* Acknowledge received, reseting it */
	if(Order_ACK_Pending)
		Order_ACK_Pending = FALSE;
}

/**
  * \fn void CMD_Handler(void)
  * \brief Gestionnaire des commandes recus
  * \return Void
 */

void CMD_Handler(void)
{
	int cmd_ack;
	if(Serial_Port_Two_Byte_Count()>0)
	{
		cmd_ack = Read_Serial_Port_Two();
		
		switch(cmd_ack&0xF0)
		{
			case CMD_DPL_ACK :
			case CMD_ENV_ACK :
				CMD_ENV_ACK_Handler(cmd_ack);
				break;
			case CMD_DPL :
				CMD_DPL_Handler(cmd_ack);
				break;
			case CMD_ENV :
				
				break;
			case CMD_ERROR :
				
				break;
			default :
				break;
		}
	}
}
	
/**
  * \fn char ENV_Data_Transmit(unsigned char Distance, char Angle)
  * \brief Méthode d'envoie des informations d'environement
  * \param[in] Distance Distance vu par le sonar
  * \param[in] Angle Angle de la tourelle
  * \return TRUE si l'envoie est effectif, FALSE sinon
 */
 	
char ENV_Data_Transmit(unsigned char Distance, char Angle)
{
	if(Order_ACK_Pending)
		return FALSE;
	
	/* No order in progress, sending ENV Data */
	
	Write_Serial_Port_Two (CMD_ENV);
	
	Write_Serial_Port_Two(Distance);
	Write_Serial_Port_Two((unsigned char)Angle);
	
	return TRUE;
}