// ****************************************************************************
//
//	fluid_ctrl.h	
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

int	 fluid_init(void);
void fluid_init_flushed(void);
int  fluid_end(void);

void fluid_tick(void);
void fluid_set_config(void);

void fluid_start_printing	(void);
void fluid_reply_stat		(RX_SOCKET socket);
void fluid_error_reset		(void);
void fluid_send_ctrlMode	(int no, EnFluidCtrlMode ctrlMode, int sendToHeads);
void fluid_send_pressure    (int no, INT32 pressure);
void fluid_send_tara        (int no);
void fluid_send_calib		(SValue *pmsg);


EnFluidCtrlMode fluid_get_ctrlMode(int no);
int  fluid_in_ctrlMode		(int no, EnFluidCtrlMode ctrlMode);
int	 fluid_purge_fluidNo	(void);
void fluid_set_head_state	(int no, SHeadStat *pstat);
INT32 fluid_get_cylinderPresSet(int no);
INT32 fluid_get_cylinderPres   (int no);
INT32 fluid_get_pumpSpeed	   (int no);
INT32 fluid_get_pumpFeedback   (int no);
// INT32 fluid_get_amcTemp        (int no);
INT32 fluid_get_error          (int no);

void undefine_PurgeCtrlMode(void);
