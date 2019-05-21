// ****************************************************************************
//
//	machine_ctrl.h	
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

int		machine_init(void);
int		machine_end(void);
int		machine_tick(void);

void	machine_reset(void);
void	machine_error_reset(void);
int		machine_set_printpar(SPrintQueueItem *pItem);
int		machine_set_scans(int scans);
UINT32	machine_get_scanner_pos(void);
int		machine_start_printing(void);
int		machine_pause_printing(void);
int		machine_stop_printing(void);
int		machine_abort_printing(void);
int		machine_clean(void);
