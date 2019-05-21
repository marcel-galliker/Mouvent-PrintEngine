// ****************************************************************************
//
//	DIGITAL PRINTING - EEprom.h
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once
#include "rx_common.h"
#include "rx_def.h"


void eeprom_init_data(int headNo, BYTE *eeprom, SHeadEEpromInfo *pInfo);

void eeprom_display(void);