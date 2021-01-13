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
#define WEB_OFFSET_VERSO 5000

#define UnitID		"Application.GUI_00_001_Main"

int	 plc_init(void);
int  plc_end(void);
int	plc_in_simu(void);
void plc_reset(void);
void plc_error_reset(void);
int  plc_set_printpar(SPrintQueueItem *pItem);
UINT32 plc_get_scanner_pos(void);
int  plc_start_printing(void);
void  plc_spooler_start_printing();
int  plc_pause_printing(int fromGui);
void  plc_spooler_pause_printing();
int  plc_stop_printing(void);
int  plc_abort_printing(void);
int  plc_clean(void);

void	plc_load_material(char *name);

int		plc_to_purge_pos(void);
int		plc_to_wipe_pos(void);
int		plc_to_fill_cap_pos(void);
int		plc_in_fill_cap_pos(void);
int		plc_to_cap_pos(void);
int		plc_in_cap_pos(void);
int		plc_in_purge_pos(void);
int		plc_in_wipe_pos(void);
int		plc_is_splicing(void);
double	plc_get_step_dist_mm(void);

int plc_handle_gui_msg(RX_SOCKET socket, void *pmsg, int len);

EnPlcState plc_get_state(void);