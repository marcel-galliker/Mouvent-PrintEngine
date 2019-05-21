// ****************************************************************************
//
//	DIGITAL PRINTING - firepulse.h
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

#define VOLTAGE_SIZE	4096

int fp_init(int memId);
int fp_end (void);

int fp_set_waveform(int no, int voltageCnt, UINT16 voltage[VOLTAGE_SIZE]);