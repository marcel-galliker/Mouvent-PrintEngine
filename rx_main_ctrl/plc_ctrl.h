// ****************************************************************************
//
//	plc_ctrl.h	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

#define WEB_OFFSET 132	// mm to add to alsolute web distance
#define WEB_OFFSET_VERSO 6000

int	 plc_init(void);
int  plc_end(void);
void plc_reset(void);
void plc_error_reset(void);
int  plc_set_printpar(SPrintQueueItem *pItem);
UINT32 plc_get_scanner_pos(void);
int  plc_start_printing(void);
int  plc_pause_printing(void);
int  plc_stop_printing(void);
int  plc_abort_printing(void);
int  plc_clean(void);

int		plc_to_purge_pos(void);
int		plc_to_wipe_pos(void);
int		plc_in_cap_pos(void);
int		plc_in_purge_pos(void);
int		plc_in_wipe_pos(void);
double	plc_get_step_dist_mm(void);


int plc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);