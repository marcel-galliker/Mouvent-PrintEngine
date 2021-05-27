// ****************************************************************************
//
//	DIGITAL PRINTING - ctr.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

void ctr_init(void);
void ctr_tick(void);
void ctr_reset(void);
void ctr_calc_reset_key(char *machineName, UCHAR *key);
void ctr_print_reset_key(char *machineName);
void ctr_add(int mm, UINT32 colors);
// void ctr_save(void);
void ctr_reset_jobLen(void);