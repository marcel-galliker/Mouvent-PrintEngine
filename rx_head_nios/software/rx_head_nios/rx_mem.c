/******************************************************************************/
/** rx_mem.c
 **
 **
 **
 **	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

#include "rx_mem.h"

static int 			  _MemSize=0;
static unsigned char *_Mem;

//--- mem_init --------------------------------------
void mem_init(void *mem, int size)
{
	_MemSize = size;
	_Mem	 = mem;
};

//--- mem_available ----------------------------------
int	  mem_available(void)
{
	return _MemSize;
};

//--- mem_alloc -------------------------------------
void *mem_alloc(int size)
{
	if (size>_MemSize)
		return 0;

	void *ptr=_Mem;
	_MemSize -= size;
	_Mem = &_Mem[size];
	return ptr;
}

