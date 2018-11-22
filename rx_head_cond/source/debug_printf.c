/**************************************************************************/ /**
  \file i2c_bitbang.h

  Debug printf over ulink-me

  \author 	Stefan Weber

  \copyright 2017 by radex AG, Switzerland. All rights reserved.
 ******************************************************************************/

#include "debug_printf.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever needed */ };
FILE __stdout;
FILE __stdin;

/**
 * \brief Function needed for printf
 *
 * Attention: This function is not reentrant and will crash if used from
 * interrupts.
 **/
int fputc(int ch, FILE *f)
{
    if (DEMCR & TRCENA)
    {
        while (ITM_Port32(0) == 0)
            ;
    ITM_Port8(0) = ch;
  }
  return(ch);
}
