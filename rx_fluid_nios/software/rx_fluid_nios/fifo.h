/******************************************************************************/
/** fifo.h
 ** 
 ** uart functions to communicate between fpga and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __fifo_h__
#define __fifo_h__

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

#define FIFO_SIZE	1024

typedef struct
{
		int wrIdx;
		int rdIdx;
		char data[FIFO_SIZE];
} SFifo;
	
void fifo_init	(SFifo *pfifo);
int	 fifo_put   (SFifo *pfifo, UCHAR *data, int count);
int	 fifo_get   (SFifo *pfifo, UCHAR *data);


#endif /* __fifo_h__ */
