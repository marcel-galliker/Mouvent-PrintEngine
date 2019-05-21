/******************************************************************************/
/** \file uart.h
 ** 
 ** uart functions to communicate between fpga and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/
 
#ifndef __uart_h__
#define __uart_h__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "rx_ink_common.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/

void uart_init	(UINT32 buadrate);
void uart_reset	(void);
void uart_write (UCHAR *data, int count);
int  uart_read  (UCHAR *data);


#endif /* __uart_h__ */
