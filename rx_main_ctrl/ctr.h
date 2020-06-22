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
void ctr_calc_reset_key(char *key);
void ctr_add(int mm);
// void ctr_save(void);
void ctr_reset_jobLen(void);