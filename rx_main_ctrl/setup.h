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

void setup_save_config();
void setup_read_config();

BOOL setup_write_config();

int setup_network	 (HANDLE file, SRxNetwork *pnet,				EN_setup_Action action);
int setup_print_queue(HANDLE file, SPrintQueueItem *item, int *cnt, EN_setup_Action action);

int setup_fluid_system	(const char *filepath,	UINT32				*pflushed,		  EN_setup_Action  action);
int setup_screw_positions(const char *filepath, SScrewPositions ppos[MAX_STEPPERS], EN_setup_Action action);

