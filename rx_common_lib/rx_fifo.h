// ****************************************************************************
//
//	rx_fifo.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

/* --- Prototypes ------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

int	rx_fifo_create		(HANDLE *hFifo, int size);
int	rx_fifo_close		(HANDLE hFifo);
int	rx_fifo_put			(HANDLE hFifo, void *buffer, int len);					// blocking, waitfor a free place
int	rx_fifo_get			(HANDLE hFifo, void **buffer, int *len, int timeout);	// buffer = NULL, timeout or nothing to get
int	rx_fifo_count		(HANDLE hFifo);											// -1: error, else count
int	rx_fifo_remain		(HANDLE hFifo);											// -1: error, else free remain


#ifdef __cplusplus
}
#endif

