// ****************************************************************************
//
//	rx_mem.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#ifdef __cplusplus
extern "C"{
#endif

UINT64	rx_mem_physical(void);

void  rx_mem_init	(UINT64 size);
BYTE* rx_mem_alloc	(UINT64 size);
int   rx_mem_use	(BYTE *ptr);		// the memory has a counter, rx_use increments the counter
int   rx_mem_cnt	(BYTE *ptr);		// return the counter
int   rx_mem_use_clear(BYTE *ptr);		// resetes the used counter
int   rx_mem_unuse	(BYTE **ptr);		// rx_mem_unuse decrements the counter
void  rx_mem_free	(BYTE **ptr);		// rx_free frees it when the counter is zero
void  rx_mem_free_force	(BYTE **ptr);		// rx_free frees also when counter is nuôn zero
int  rx_mem_await_free(BYTE *ptr, int timeout);
int  rx_mem_await_abort(BYTE *ptr);

UINT32 rx_mem_get_totalMB(void);
UINT32 rx_mem_get_freeMB(void);
void   rx_mem_clear_caches(void);

UINT64 rx_mem_allocated();

#ifdef __cplusplus
}
#endif

