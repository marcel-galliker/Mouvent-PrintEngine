// ****************************************************************************
//
//	screening.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************


#pragma once

#include "rx_common.h"


void scr_init(void);
void scr_end(void);
void scr_abort(void);
void scr_malloc(UINT32 lengthPx, UINT8 bitsPerPixel);
void scr_set_values(int head, INT16 values[MAX_DENSITY_VALUES]);
void scr_wait_ready(void);
void scr_start(SBmpSplitInfo *pInfo);
int  scr_wait(int timeout);
