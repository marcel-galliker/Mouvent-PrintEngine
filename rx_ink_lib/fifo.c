/******************************************************************************/
/** \file fifo.c
 ** 
 ** FIFO for handling UART.
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include "fifo.h"

//--- prototypes --------------------------------------------------------------static void InitMfs0(UINT32 baudrate);

//--- fifo_init ---------------------------------
void fifo_init(SFifo *pfifo)
{
	memset(pfifo, 0, sizeof(SFifo));
}

//--- fifo_put -------------------------------------
int fifo_put(SFifo *pfifo, UCHAR *data, int count)
{
	int idx;
	if (!count) return FALSE;
	for(idx = pfifo->wrIdx; count; count--, data++)
	{
		pfifo->data[idx++] = *data;
		if (idx>=FIFO_SIZE) idx=0;
		if (idx==pfifo->rdIdx) 
			return FALSE;
	}
	pfifo->wrIdx = idx;
	return TRUE;
}

//--- fifo_get -----------------------------------
int fifo_get(SFifo *pfifo, UCHAR *data)
{
	if (pfifo->rdIdx!=pfifo->wrIdx)
	{
		int idx = pfifo->rdIdx+1;
		*data = pfifo->data[pfifo->rdIdx];
		if (idx<FIFO_SIZE)
            pfifo->rdIdx = idx;
		else			   
            pfifo->rdIdx = 0;
		return TRUE; 
	}
	return FALSE;
}
