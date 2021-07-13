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

int	 step_active(int no);

int	 step_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int	 step_do_test(SStepperMotorTest *pmsg);
void step_abort_printing(void);

void step_lift_to_wipe_pos(EnFluidCtrlMode mode);
int  step_lift_in_wipe_pos(EnFluidCtrlMode mode);

void step_lift_to_print_pos(void);
int  step_lift_in_print_pos(void);

void step_lift_to_top_pos(void);
int  step_lift_in_top_pos(int stepperno);

void step_lift_to_up_pos(void);
int  step_lift_in_up_pos(void);

void step_lift_stop(void);

void step_set_vent(int speed);

int  step_rob_reference_done(void);
void step_rob_do_reference(void);

void step_rob_to_center_pos(void);

void step_rob_to_wipe_pos(ERobotFunctions rob_function);
int  step_rob_in_wipe_pos(ERobotFunctions rob_function);

void step_rob_wipe_start(EnFluidCtrlMode mode);
int  step_rob_wipe_done(EnFluidCtrlMode mode);

void step_rob_stop(void);
void step_empty_waste(int time);

void step_set_autocapMode(int state);
	
UINT32 tt_get_scanner_pos(void);
int  tt_set_printpar   (SPrintQueueItem *pItem);
int  tt_set_scans	   (int scans);
int  tt_start_printing (void);
int  tt_pause_printing (void);
int  tt_stop_printing  (void);
int  tt_abort_printing (void);
int  tt_clean		   (void);
int	 tt_cap_to_print_pos(void);
void step_adjust_heads(RX_SOCKET socket, SHeadAdjustmentMsg* headAdjustment);
void step_robi_to_garage(RX_SOCKET socket);
SStepperStat step_get_StepperStatus(SHeadAdjustmentMsg *headAdjustment);
int step_screw_in_Buffer(SHeadAdjustmentMsg *headAdjustment);
int step_get_stitch_position(SHeadAdjustmentMsg *headAdjustment);
int step_robot_used(int fluidNo);
int step_stepper_to_fluid(int fluidNo);
int step_stepper_to_cluster(int clusterNo);
int step_get_ScrewPos(int stepperNo);
