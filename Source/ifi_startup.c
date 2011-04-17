/**
 * \file ifi_startup.c
 * \brief Startup Code - Fournis par Vex - Ne pas modifier 
 * \author Vex Robotics
 */ 
 
/*******************************************************************************
* FILE NAME: ifi_startup.c
*
* DESCRIPTION:
*  This file contains important startup code.
*
* USAGE:
*  This file should not be modified at all by the user.
*
*  DO NOT MODIFY THIS FILE!
*******************************************************************************/

#include "../Header/ifi_default.h"

extern void Clear_Memory (void);
extern void main (void);

void _entry (void);     /* prototype for the startup function */
void _startup (void);
void _do_cinit (void);  /* prototype for the initialized data setup */

extern volatile near unsigned long short TBLPTR;
extern near unsigned FSR0;
extern near char FPFLAGS;
#define RND 6

#pragma code _entry_scn=RESET_VECTOR
void _entry (void)
{
_asm goto _startup _endasm

}

#pragma code _startup_scn
void _startup (void)
{
  _asm
    /* Initialize the stack pointer */
    lfsr 1, _stack lfsr 2, _stack clrf TBLPTRU, 0 /* 1st silicon doesn't do this on POR */
    bcf  FPFLAGS,RND,0 /* Initialize rounding flag for floating point libs */
    
    /* initialize the flash memory access configuration. this is harmless */
    /* for non-flash devices, so we do it on all parts. */
    bsf 0xa6, 7, 0
    bcf 0xa6, 6, 0
  _endasm 

loop:

 	Clear_Memory();              
  _do_cinit ();
  /* Call the user's main routine */
  main ();

  goto loop;
}                               /* end _startup() */

/* MPLAB-C18 initialized data memory support */
/* The linker will populate the _cinit table */
extern far rom struct
{
  unsigned short num_init;
  struct _init_entry
  {
    unsigned long from;
    unsigned long to;
    unsigned long size;
  }
  entries[];
}
_cinit;

#pragma code _cinit_scn
void
_do_cinit (void)
{
  /* we'll make the assumption in the following code that these statics
   * will be allocated into the same bank.
   */
  static short long prom;
  static unsigned short curr_byte;
  static unsigned short curr_entry;
  static short long data_ptr;

  /* Initialized data... */
  TBLPTR = (short long)&_cinit;
  _asm
    movlb data_ptr
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf curr_entry, 1
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf curr_entry+1, 1
  _endasm
    test:
  _asm
     bnz 3
    tstfsz curr_entry, 1
    bra 1
  _endasm
  goto done;
    /* Count down so we only have to look up the data in _cinit
     * once.
     *
     * At this point we know that TBLPTR points to the top of the current
     * entry in _cinit, so we can just start reading the from, to, and
     * size values.
     */
  _asm
  /* read the source address */
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf prom, 1
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf prom+1, 1
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf prom+2, 1
    /* skip a byte since it's stored as a 32bit int */
    tblrdpostinc
    /* read the destination address directly into FSR0 */
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf FSR0L, 0
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf FSR0H, 0
    /* skip two bytes since it's stored as a 32bit int */
    tblrdpostinc
    tblrdpostinc
    /* read the destination address directly into FSR0 */
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf curr_byte, 1
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf curr_byte+1, 1
    /* skip two bytes since it's stored as a 32bit int */
    tblrdpostinc
    tblrdpostinc
  _endasm  

  /* the table pointer now points to the next entry. Save it
   * off since we'll be using the table pointer to do the copying
   * for the entry.
   */
  data_ptr = TBLPTR;

  /* now assign the source address to the table pointer */
  TBLPTR = prom;

  /* do the copy loop */
  _asm
          /* determine if we have any more bytes to copy */
    movlb curr_byte
    movf  curr_byte, 1, 1
copy_loop:
    bnz 2 /* copy_one_byte */
    movf  curr_byte + 1, 1, 1
    bz 7  /* done_copying */

copy_one_byte:
    tblrdpostinc
    movf  TABLAT, 0, 0
    movwf POSTINC0, 0

    /* decrement byte counter */
    decf  curr_byte, 1, 1
    bc -8   /* copy_loop */
    decf  curr_byte + 1, 1, 1
    bra -7  /* copy_one_byte */

done_copying:
  _endasm
      /* restore the table pointer for the next entry */
  TBLPTR = data_ptr;
  /* next entry... */
  curr_entry--;
  goto test;
done:
;
}
