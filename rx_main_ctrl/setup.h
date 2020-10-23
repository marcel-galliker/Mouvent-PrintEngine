// ****************************************************************************
//
//	DIGITAL PRINTING - Setup.h
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
#include "rx_setup_file.h"

void setup_test();

int setup_network	 (HANDLE file, SRxNetwork *pnet,				EN_setup_Action action);
int setup_print_queue(HANDLE file, SPrintQueueItem *item, int *cnt, EN_setup_Action action);

int setup_config		(const char *filepath, SRxConfig			*pcfg,			  EN_setup_Action  action);
int setup_fluid_system	(const char *filepath,	UINT32				*pflushed,		  EN_setup_Action  action);
int setup_screw_positions (const char *filepath, SRxConfig			*pcfg,			  EN_setup_Action  action);
int setup_vacuum_cleaner(const char* filepath, INT32 *time, EN_setup_Action action);
