/******************************************************************************/
/** \file comm.h
 ** 
 ** Communication between NIOS and Conditioner
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

#ifndef __COMM_H__
#define __COMM_H__

#include "rx_ink_common.h"

#define COMM_BUF_SIZE	512
//#define COMM_BUF_SIZE	1024

//--- defines -----------------------------------------------------------------

void comm_init(int commNo);

int  comm_encode(void *pin, int inlen, UCHAR *pbuf, int bufsize, int *pbuflen);

int  comm_received(int commNo, char ch);
	// return: TRUE= valid data received

int	 comm_get_data(int commNo, UCHAR *pbuf, int bufsize, int *pbuflen);

#endif /* __COMM_H__ */
