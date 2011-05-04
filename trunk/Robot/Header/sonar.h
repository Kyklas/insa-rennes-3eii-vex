/**
 * \file sonar.h
 * \brief Definition du driver pour le sonar
 * \author Stanislas BERTRAND
 * \version 1.0
 * \date 4 avril 2010
 */ 
#ifndef _SONAR_H
#define _SONAR_H


// timer 1 definition

#define T1_16BIT 	0x80
#define T1_PS1 		0x00
#define T1_PS2 		0x10
#define T1_PS4 		0x20
#define T1_PS8 		0x30
#define T1_INTCK	0x00
#define T1_OCK		0x02

#define T1_ON  		T1CONbits.TMR1ON

#define T1_IP		IPR1bits.TMR1IP
#define T1_IF		PIR1bits.TMR1IF
#define T1_IE		PIE1bits.TMR1IE

// Interrupt 1 definition
// Interrupt 1 -> INT2
#define INT1_PRY		INTCON3bits.INT2IP
#define INT1_FLG		INTCON3bits.INT2IF
#define INT1_ENBL		INTCON3bits.INT2IE
#define INT1_EDG		INTCON2bits.INTEDG2

// Digital Out for pulse
#define SONAR_IO_OUT		IO2
#define SONAR_PULSE_OUT		rc_dig_out02

extern unsigned int SonarDistance;
extern short ValidSonarDistance;

void SonarInit(void);
void SonarHandle(void);
void SonarPulse(void);

#endif