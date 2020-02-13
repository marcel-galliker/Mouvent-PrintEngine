// ****************************************************************************
//
//	DIGITAL PRINTING - motor.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "power_step.h"
#include "fpga_def_stepper.h"

#define L5918_STEPS_PER_METER	3200	// LBxxx: Motor steps/meter
#define L5918_INC_PER_METER		16000	// LBxxx: Encoder incs/meter

// max speeds LBxx
// current=200	max_speed=20'000
// current=300	max_speed=16'000
// current=420	max_speed=13'000


#define L3518_STEPS_PER_METER	3200	// TXxxx: Motor steps/meter
#define L3518_INC_PER_METER		4000	// TXxxx: Encoder incs/meter

#define ESTOP_UNUSED	15	// input for uniused E-STOP

#define MICROSTEPS		1
#define STEPS			16

// max speeds TXxx
// current=???	max_speed=???

typedef enum
{
	chk_off,		// 00
	chk_std,		// 01
	chk_txrob_ref,	// 02
	chk_txrob,		// 03
	chk_lbrob,		// 04
	chk_lb_ref1,	// 05
	chk_lb_ref2,	// 06
    chk_txrob_ref2, // 07
} EEncCheck;
	
typedef struct
{
	INT32	speed;	// Hz
	UINT32	accel;	// Hz/Sec
	double  current_acc;
	double  current_run;
	UINT32	stop_mux;
	INT32	estop_in_bit[MOTOR_CNT];
	INT32	estop_level;
	INT32	dis_mux_in;
	EEncCheck encCheck;
	UINT32	enc_bwd;
} SMovePar;

void	motor_init(void);
void	motor_end (void);

void	motor_main(int ticks, int menu);
int		motors_init_done(void);


void	motor_trace_move(int motor);

void	motor_config (int motor, int currentHold, double stepsPerMeter, double incPerMeter, int steps);
void	motors_config(int motors, int currentHold, double stepsPerMeter, double incPerMeter, int steps);

INT32	motor_get_step(int motor);
INT32	motor_get_end_step(int motor);
int     motor_move_to_step(int motor, SMovePar *par, INT32 steps);
int     motors_move_to_step(int motor, SMovePar *par, INT32 steps);
int		motor_move_by_step(int motor, SMovePar *par, INT32 steps);
int		motors_move_by_step(int motors, SMovePar *par, INT32 steps, UINT32 errorCheck);
int		motors_quad_move_by_step(int motor, SMovePar *par[4], INT32 steps, UINT32 errorCheck);

void	motor_reset(int motor);
void	motors_reset(int motors);
void	motors_start(UINT32 motors, UINT32 errorCheck);	// bitset of motors
void	motors_stop(UINT32 motors);	// bitset of motors
void	motors_estop(UINT32 motors);	// bitset of motors
void    motor_set_hold_current(int motor);
int		motor_move_done(int motor);
int		motors_move_done(int motors);
int		motor_error(int motor);
int		motors_error(int motors, int *err); // checks all motors: return "TRUE=ERROR", err=motornumber of error
