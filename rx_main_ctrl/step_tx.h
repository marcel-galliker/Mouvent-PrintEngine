// ****************************************************************************
//
//	step_tx.h		stepper for textile machines
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_sok.h"

void steptx_init(int stepperNo, RX_SOCKET *psocket, UINT32 ipaddr);
void steptx_error_reset();

int	 steptx_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steptx_handle_status(int stepperNo, STestTableStat *pStatus);

int	 steptx_to_print_pos(void);
int	 steptx_to_up_pos(void);

int	 steptx_slide_enabled(void);

int	 steptx_to_purge_pos(int no);
int  steptx_in_purge_pos(void);

int	 steptx_wipe_start(int no);
int	 steptx_wipe_done(void);
