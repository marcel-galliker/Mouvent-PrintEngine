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

void steptx_init(int stepperNo, RX_SOCKET psocket);
void steptx_error_reset();

int	 steptx_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steptx_handle_status(int stepperNo, SStepperStat *pStatus);

void steptx_lift_to_wipe_pos(EnFluidCtrlMode mode);
int  steptx_lift_in_wipe_pos(EnFluidCtrlMode mode);

void steptx_lift_to_print_pos(void);
int	 steptx_lift_in_print_pos(void);

void steptx_lift_to_up_pos(void);
int	 steptx_lift_in_up_pos(void);

void steptx_lift_stop(void);

void steptx_rob_do_reference(void);
int  steptx_rob_reference_done(void);

void steptx_rob_to_center_pos(void);

void steptx_rob_to_wipe_pos(ERobotFunctions rob_function);
int  steptx_rob_in_wipe_pos(ERobotFunctions rob_function);

void steptx_rob_wipe_start(EnFluidCtrlMode mode);
int	 steptx_rob_wipe_done(EnFluidCtrlMode mode);

void _steptx_rob_vacuum_start(void);
	
void steptx_rob_wash_start(void);
int	 steptx_rob_wash_done(void);

void steptx_rob_stop(void);

void steptx_set_robCtrlMode(EnFluidCtrlMode ctrlMode);

EnFluidCtrlMode state_RobotCtrlMode(void);
