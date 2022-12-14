// ****************************************************************************
//
//	DIGITAL PRINTING - tse.h
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

int  tse_init(void *addr0, void *addr1);
int  tse_end(void);

void tse_display_status(INT32 speed[2]);

int  tse_set_mac_addr(int no, UINT64 macAddr);
int  tse_check_errors(int ticks, int menu);
void tse_error_reset(void);
