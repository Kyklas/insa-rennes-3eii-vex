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
 
#include "../Header/orders.h"
#include "../Header/serial_ports.h"
#include "../Header/communication.h"
#include "../Header/ifi_picdefs.h"
#include <stdio.h>

 /* ------------------------------------ */
 /* -------- Variables globales -------- */
 /* ------------------------------------ */

/**
 * \var Order_Turret_Angle
 * \brief Consigne de gisement de la tourelle sonar
 * Variable globale declaree dans user_routine_fast.c
*/
 volatile char Order_Turret_Angle ;

/**
 * \var Order_Motor_Left
 * \brief Consigne de vitesse du moteur gauche
 * Variable globale declaree dans user_routine_fast.c
*/
extern volatile unsigned char Order_Motor_Left;

/**
 * \var Order_Motor_Right
 * \brief Consigne de vitesse du moteur droit
 * Variable globale declaree dans user_routine_fast.c
*/
extern volatile unsigned char Order_Motor_Right;


/**
 * \var Rx_2_Queue_Byte_Count
 * \brief Nb d'octet dans la file d'attente de reception du port serie 2
 * Variable globale declaree dans serial_port.c
*/
extern volatile unsigned char Rx_2_Queue_Byte_Count;

 /* --------------------------- */
 /* -------- Fonctions -------- */
 /* --------------------------- */

/**
 * \fn void Cmd_Ack (unsigned char Cmd)
 * \brief Envoi un acquittement sur la commande recue au PDA
 * \param[in] Cmd Dernier ordre recu
 * \return Void
*/
void Cmd_Ack (unsigned char Cmd)
{
	switch (Cmd&0xF0) /* Masque les 4 LSB de la commande*/
	{	/* Décodage de la commande et envoi de l'acquittement correspondant */
		case CMD_DPL : Write_Serial_Port_Two (CMD_DPL_ACK); break;
		case CMD_ENV : Write_Serial_Port_Two (CMD_ENV_ACK); break;
		/* Si la commande n'est pas répertoriée, envoyer CMD_ERROR */
		default : Write_Serial_Port_Two (CMD_ERROR);
	}
	
	/* Traitement l'interruption sur TX2 */
	Tx_2_Int_Handler();
}


/**
 * \fn void Order_Turret_Angle_Update (char Order_Turret)
 * \brief Modifie la consigne de gisement de la tourelle
 * \param[in,out] Order_Turret Consigne de gisement de la tourelle
 * \return Void
*/
void Order_Turret_Angle_Update (char Order_Turret)
{
	/* Met à jour la consigne de gisement de la tourelle */
	Order_Turret_Angle = Order_Turret;  
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
 * \fn unsigned char Cmd_Receive (void)
 * \brief Procedure de reception d'une commande envoyee par le PDA
 * \param Void
 * \return La commande sur un octet
*/
unsigned char Cmd_Receive (void)
{
	unsigned char CMD;
	unsigned char  byte1, byte2, temp;
	
	/* Désactive les interruptions sur RX2 */
	PIE3bits.RC2IE = 0;
	
	/* Lit la commande reçue sur le port série 21 */
	CMD = Read_Serial_Port_Two();
	
	/* Désactive les interruptions sur RX2 (ré-activée par Read_Serial_Port_Two() ) */
	PIE3bits.RC2IE = 0;
	
	/* Envoi de l'acquittement sur la commande */
	Cmd_Ack (CMD);
	
	/* Command bits mask */
	temp = CMD & 0xF0;
	
	switch (temp) 
	{	/* Si la commande reçue est une commande de déplacement */
		case CMD_DPL :
		{	/* Attend la réception d'un nouvel octet sur le port série 2 */
			while (!PIR3bits.RC2IF);
			
			/* Traite l'interruption sur le port série 2 */
			Rx_2_Int_Handler();
			
			/* Attend la réception d'un nouvel octet sur le port série 2 */
			while (!PIR3bits.RC2IF);
			
			/* Traite l'interruption sur le port série 2 */
			Rx_2_Int_Handler();
			
			/* Les deux octets sont reçus, on peut les lire */
			byte1 = Read_Serial_Port_Two();
			byte2 = Read_Serial_Port_Two();
			
			/* Mise à jour des consignes moteurs */
			Order_Motor_Left_Update (byte1);
			Order_Motor_Right_Update (byte2);	
			
			break;
		}
		/* Si la commande reçue est une commande d'environnement */
		case CMD_ENV :  
		{	/* Attend la réception d'un nouvel octet sur le port série 2 */
			while (!PIR3bits.RC2IF);
			
			/* Traite l'interruption sur le port série 2 */
			Rx_2_Int_Handler();
			
			/* Octet reçu, on peut le lire */
			byte1 = (char) Read_Serial_Port_Two();	

			/* Mise à jour de la consigne d'angle de la tourelle */
			Order_Turret_Angle_Update (byte1);
			
			break;
		}
	}
	
	/* Active les interruptions sur le port série 2 */
	PIE3bits.RC2IE = 1;
	
	/* Retourne la commande reçue */
	return (CMD);	
}


unsigned char ENV_Data_Transmit (unsigned char Distance, char Angle)
{
	unsigned char Ack;
	
	/* Envoi l'octet de commande d'environnement */
	Write_Serial_Port_Two(CMD_ENV);
	
	/*Attente de la reception de l'acknowledge*/
	while(!(Serial_Port_Two_Byte_Count()>=1));
	
	/* Lecture de l'acquittement */
	Ack = Read_Serial_Port_Two();
	printf((char *)"ENV_Data_Transmit Ack : %X\n\r",Ack);
	/* Si l'acquittement reçu correspond à un acquittement de commande d'environnement */
	if (Ack == CMD_ENV_ACK)
	{
		Write_Serial_Port_Two(Distance);
		Write_Serial_Port_Two((unsigned char)Angle);
				
		/* Attend que le buffer d'envoi soit vide */
		while(PIE3bits.TX2IE);
		
		return TRUE;
	}
	else
	{
		Write_Serial_Port_Two(CMD_ERROR);
		
		/* Attend que le buffer d'envoi soit vide */
		while(PIE3bits.TX2IE);
		
		return FALSE;
	}
}	

/**
 * \fn unsigned char Order_ENV_Transmit (unsigned char Distance, char Angle)
 * \brief Procedure d'envoi des infos d'environnement au PDA
 * \param[in] Distance Distance de l'objet le plus proche pour un angle donne
 * \param[in] Angle Angle de gisement de la tourelle
 * \return TRUE si l'acquittement du PDA est OK, FALSE sinon
*/
unsigned char Order_ENV_Transmit (unsigned char Distance, char Angle)
{
	unsigned char Ack;
	
	/* Désactive les interruptions sur RX2*/
	PIE3bits.RC2IE = 0;
	
	/* Envoi l'octet de commande d'environnement */
	Write_Serial_Port_Two(CMD_ENV);
	
	/* Attend la réception d'un nouvel octet sur le port série 2 */
	while (!PIR3bits.RC2IF);
	
	/* Traite l'interruption sur le port série 2 */
	Rx_2_Int_Handler();
	
	/* Lecture de l'acquittement */
	Ack = Read_Serial_Port_Two();
		
	/* Si l'acquittement reçu correspond à un acquittement de commande d'environnement */
	if (Ack == CMD_ENV_ACK)
	{
		/* Envoi l'octet de correspondant à la distance mesurée */
		Write_Serial_Port_Two(Distance);
		
		/* Attend que le buffer d'envoi soit vide */
		while(!PIR3bits.TX2IF);
				
		/* Envoi l'octet de correspondant au gisement de la tourelle */
		Write_Serial_Port_Two((unsigned char)Angle);
				
		/* Attend que le buffer d'envoi soit vide */
		while(!PIR3bits.TX2IF);
		
		/* Active les interruptions sur le port série 2 */
		PIE3bits.RC2IE = 1;	
		
		/* Retourne TRUE */
		return (TRUE);
	}
	else
	{
		/* Envoie CMD_ERROR au PDA */
		Write_Serial_Port_Two (CMD_ERROR);
		
		/* Attend que le buffer d'envoi soit vide */
		while(!PIR3bits.TX2IF);
		
		/* Retourne FALSE */
		return (FALSE);
	}
}

	