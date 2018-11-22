// ****************************************************************************
//
//	print_ctrl.h	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"

int	pc_init(void);
int pc_end(void);

int pc_start_printing(void);
int pc_stop_printing(void);
int pc_off(void);
int pc_pause_printing(void);
int pc_abort_printing(void);
int pc_auto(int enable);
int pc_print_next(void);

int pc_sent(SPageId *id);
int pc_printed(SPageId *id, int headNo);

int pc_get_startscans(void);