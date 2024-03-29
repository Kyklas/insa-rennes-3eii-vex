/**
 * \file ifi_utilities.h
 * \brief Fonctions utiles concernant les peripheriques du uC - Fournis par Vex - Ne pas modifier 
 * \author Vex Robotics
 */ 

/*******************************************************************************
* FILE NAME: ifi_utilities.h
*
* DESCRIPTION: 
*  This is the include file which corresponds to ifi_utilities.c
*  It contains some aliases and function prototypes used in that file.
*
* USAGE:
*  This file should not be modified by the user.
*  DO NOT EDIT THIS FILE!
*******************************************************************************/

#ifndef __ifi_utilities_h_
#define __ifi_utilities_h_

#define RXINTF              PIR1bits.RCIF
#define RXINTE              PIE1bits.RCIE
#define TXINTF              PIR1bits.TXIF 
#define TXINTE              PIE1bits.TXIE
#define RCSTAbits           RCSTA1bits
#define RCSTA               RCSTA1
#define TXSTA               TXSTA1
#define TXREG               TXREG1
#define RCREG               RCREG1
#define SPBRG               SPBRG1
#define OpenUSART           Open1USART

/*******************************************************************************
                             MACRO DEFINITIONS
*******************************************************************************/

typedef enum
{
  baud_19 = 128,
  baud_38 = 64,
  baud_56 = 42,
  baud_115 = 21
} SERIAL_SPEED;


/*******************************************************************************
                           FUNCTION PROTOTYPES
*******************************************************************************/

void Hex_output(unsigned char temp);  /* located in ifi_library.lib */

  /* located in vex_library.lib */
void Generate_Pwms(unsigned char pwm_1,unsigned char pwm_2,
                   unsigned char pwm_3,unsigned char pwm_4,
                   unsigned char pwm_5,unsigned char pwm_6,
                   unsigned char pwm_7,unsigned char pwm_8);

/* These routines reside in ifi_utilities.c */
void Wait4TXEmpty(void);
void PrintByte(unsigned char odata);
void PrintWord(unsigned int odata);
void PrintString(char *bufr);
void DisplayBufr(unsigned char *bufr);
void PacketNum_Check(void);
void Initialize_Serial_Comms (void);
void Set_Number_of_Analog_Channels (unsigned char number_of_channels);
unsigned int Get_Analog_Value(unsigned char channel);

#endif


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
