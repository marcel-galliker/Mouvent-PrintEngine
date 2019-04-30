// ****************************************************************************
//
//	step_ctrl.h		Test Table Controol
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

int	 step_init(void);
int  step_end(void);

void step_tick(void);
int	 step_set_config(void);
void step_error_reset(void);

int	 step_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int	 step_do_test(SStepperMotorTest *pmsg);
int  step_abort_printing(void);

void setp_send_ctrlMode(EnFluidCtrlMode ctrlMode);
int  step_all_in_ctrlMode(EnFluidCtrlMode ctrlMode);

int	 step_to_purge_pos(int no);
int  step_in_purge_pos(void);

int  step_set_vent(int speed);

int  step_wipe_start(int no);
int  step_wipe_done(void);

UINT32 tt_get_scanner_pos(void);
int  tt_set_printpar   (SPrintQueueItem *pItem);
int  tt_set_scans	   (int scans);
int  tt_start_printing (void);
int  tt_pause_printing (void);
int  tt_stop_printing  (void);
int  tt_abort_printing (void);
int  tt_clean		   (void);

int	 tt_cap_to_print_pos(void);


