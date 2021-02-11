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
	INC_undef,		//	0
	INC_perLabel,	//	1
	INC_perRow		//  2
} EIncrement;

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
	EIncrement	increment;
} SCounterDef;

extern int					CTR_Counter;

#ifdef __cplusplus
	extern "C"{
#endif

EXPORT int  ctr_load_def(void *doc, SCounterDef *pCtrDef);
EXPORT int  ctr_save_def(void *doc, SCounterDef *pCtrDef);

EXPORT int  ctr_def_is_equal(SCounterDef *pCtrDef1, SCounterDef *pCtrDef2);
EXPORT void ctr_set_def		(SCounterDef *pCtrDef);
EXPORT EIncrement ctr_increment_mode();
EXPORT void ctr_set_counter	(INT32 recNo);
EXPORT void ctr_inc_counter	(void);
EXPORT int  ctr_get_counter	(INT32 recNo, char *str);

#ifdef __cplusplus
	}
#endif