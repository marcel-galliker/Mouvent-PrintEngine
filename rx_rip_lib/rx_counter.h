// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "export.h"
#include "rx_common.h"

typedef enum
{
	IncTypeByLineAsc = 0,	// By line in increasing order
	IncTypeByLineDec = 1,	// By line in decreasing order
	IncTypeByColAsc = 2,	// By column in increasing order
	IncTypeByColDec = 3,	// By column in decreasing order
	IncTypeByRowAsc = 4,	// By raw in increasing order (all labels on a row have the same counter value)
	IncTypeByRowDec = 5,	// By raw in decreasing order (all labels on a row have the same counter value)
} EIncType;

typedef struct
{
	INT32		size;
	char		elements_n[64];
	char		elements_a[64];
	char		elements_x[64];
	char		elements[32];
	char		start[32];
	char		end[32];
	INT32		leadingZeros;
	EIncType	incrementType;
} SCounterDef;

extern int					CTR_Counter;

#ifdef __cplusplus
	extern "C"{
#endif

EXPORT int  ctr_load_def(void *doc, SCounterDef *pCtrDef);
EXPORT int  ctr_save_def(void *doc, SCounterDef *pCtrDef);

EXPORT int  ctr_def_is_equal(SCounterDef *pCtrDef1, SCounterDef *pCtrDef2);
EXPORT void ctr_set_def		(SCounterDef *pCtrDef);
EXPORT EIncType ctr_increment_type();
EXPORT void ctr_set_counter	(INT32 recNo);
EXPORT void ctr_inc_counter	(void);
EXPORT int  ctr_get_counter	(INT32 recNo, char *str);

#ifdef __cplusplus
	}
#endif