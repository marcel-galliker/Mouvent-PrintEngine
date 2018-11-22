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

//--- 
typedef struct
{
	UINT32	stepsPerM;	// steps per meter
} SMotorCfg;

typedef struct
{
	INT32	speed;	// Hz
	UINT32	accel;	// Hz/Sec
	double  current;
	UINT32	stop_mux;
	INT32	estop_in;
			#define ESTOP_UNUSED	15	// input for uniused E-STOP
	INT32	estop_level;
	INT32	checkEncoder;
} SMovePar;

void	motor_init(void);
void	motor_end (void);

void	motor_main(int ticks, int menu);
int		motors_init_done(void);


void	motor_trace_move(int motor);

void	motor_config(int motor, int currentHold, double stepsPerMeter, double incPerMeter);
void	motors_config(int motors, int currentHold, double stepsPerMeter, double incPerMeter);

INT32	motor_get_step(int motor);
INT32	motor_get_end_step(int motor);
int     motor_move_to_step(int motor, SMovePar *par, INT32 steps);
int     motors_move_to_step(int motor, SMovePar *par, INT32 steps);
int 	motors_dual_move_to_step(int motor, SMovePar *par_pos, SMovePar *par_neg, INT32 steps);
int		motor_move_by_step(int motor, SMovePar *par, INT32 steps);
int		motors_move_by_step(int motors, SMovePar *par, INT32 steps, UINT32 errorCheck);
int		motors_quad_move_by_step(int motor, SMovePar *par[4], INT32 steps, UINT32 errorCheck);

void	motor_reset(int motor);
void	motors_reset(int motors);
void	motors_start(UINT32 motors, UINT32 errorCheck);	// bitset of motors
void	motors_stop (UINT32 motorsk);	// bitset of motors
void	motors_estop(UINT32 motorsk);	// bitset of motors
int		motor_move_done(int motor);
int		motors_move_done(int motors);
int		motor_error(int motor);
int		motors_error(int motors, int *err); // checks all motors: return "TRUE=ERROR", err=motornumber of error
