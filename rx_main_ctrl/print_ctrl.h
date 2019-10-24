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

int  pc_start_printing(void);
int  pc_stop_printing(int userStop);
int  pc_off(void);
int  pc_pause_printing(int fromGui);
int  pc_abort_printing(void);
int  pc_auto(int enable);
int  pc_print_next(void);
void pc_set_pageMargin(INT32 pageMargin);

int  pc_sent(SPageId *id);
void pc_print_go(void);
int  pc_print_done(int headNo, SPrintDoneMsg *pmsg);
void pq_next_page(SPrintQueueItem *pitem, SPageId *pid);

void pc_del_file(char *path);

int pc_get_startscans(void);