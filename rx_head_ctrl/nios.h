// ****************************************************************************
//
//	DIGITAL PRINTING - nios.h
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "nios_def_head.h"

void nios_shutdown(void);

int  nios_init(void);
int  nios_end(void);
void nios_load(const char *exepath);
int  nios_loaded(void);

int  nios_main(int ticks, int menu);
void nios_check_errors(void);
void nios_error_reset(void);

void nios_setInk(int headNo, SInkDefinition *pink, int maxDropSize, int fpVoltage);

void nios_fixed_grey_levels(int fixedDropSize, int maxDropSize);
// void nios_display_status(int nios, int status, int eeprom);
void nios_udpBlockSize(UINT32 blockSize);

void toggle_watchdog_mode(void);

//--- external variables ---------------
extern int	NIOS_FixedDropSize;
extern int	NIOS_Droplets;

extern SNiosCfg		*_NiosCfg;	
extern SNiosStat	*_NiosStat;
extern SNiosMemory	*_NiosMem;