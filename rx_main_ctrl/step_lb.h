// ****************************************************************************
//
//	step_lb.h		Test Table Controol
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

void steplb_init(int no, RX_SOCKET psocket);

int	 steplb_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steplb_handle_status(int no, SStepperStat *pStatus);
int	 steplb_set_ScrewPos(int no, SScrewPositions *ppos);
int steplb_get_ScrewPos(int stepperNo);

int	 steplb_to_print_pos(void);
void steplb_prevent_dripping(int clusterNo);
void steplb_abort_printing(void);

void steplb_lift_to_top_pos_all(void);
void steplb_lift_to_top_pos(int stepperNo);
int steplb_lift_in_top_pos_all(void);
int steplb_lift_in_top_pos(int stepperNo);

void steplb_lift_to_up_pos_all(void);
int	 steplb_lift_in_up_pos_all(void);

void steplb_lift_to_up_pos(int no);
int	 steplb_lift_in_up_pos(int no);

void steplb_lift_to_fct_pos(int no, ERobotFunctions rob_function); 
int  steplb_lift_in_fct_pos(int no, ERobotFunctions rob_function);

void steplb_rob_to_fct_pos(int no, ERobotFunctions rob_function);
void steplb_rob_to_fct_pos_all(ERobotFunctions rob_function);
int	 steplb_rob_in_fct_pos(int no, ERobotFunctions rob_function);
int	 steplb_rob_in_fct_pos_all(ERobotFunctions rob_function);

void steplb_rob_fct_start(int no, ERobotFunctions rob_function);
int	 steplb_rob_fct_done (int no, ERobotFunctions rob_function);

void steplb_rob_do_reference(void);
void steplb_rob_empty_waste_all(int time_s);
void steplb_rob_empty_waste(int stepperNo, int time_s);
int  steplb_rob_reference_done(void);

void steplb_rob_stop_all(void);
void steplb_rob_stop(int no);

void steplb_rob_control_all(EnFluidCtrlMode ctrlMode);
void steplb_rob_control(EnFluidCtrlMode ctrlMode, int no);
void steplb_rob_start_cap_all(void);

int	steplb_printbarUsed(int stepperNo);

void steplb_adjust_heads(RX_SOCKET socket, SHeadAdjustmentMsg *headAdjustment);

// SStepperStat steplb_get_StepperStatus(SHeadAdjustmentMsg *headAdjustment);
int steplb_get_stitch_position(SHeadAdjustmentMsg *headAdjustment);

void steplb_pump_back_fluid(int fluidNo, int state);
void steplb_set_autocapMode(int state);

void steplb_set_fluid_off(int no);

int steplb_robot_used(int fluidNo);
int steplb_stepper_to_fluid(int fluidno);
int steplb_stepper_to_cluster(int clusterNo);
