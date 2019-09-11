// ****************************************************************************
//
//	DIGITAL PRINTING - motor.c
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "rx_error.h"
#include "rx_threads.h"
#include "fpga_def_stepper.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "test_table.h"
#include "math.h"
#include "motor_cfg.h"
#include "motor.h"

//--- defines ----------------------------------------------------------------
#define FOR_ALL_MOTORS(motor, motors) for (motor=0; motor<MOTOR_CNT; motor++) if (motors & (1<<motor))

#define MIN_SPEED_HZ	  100		// minimal speed in Hz
#define MAX_SPEED_HZ	17000		// maximal speed in Hz

#define FIX_POINT		0x10000

//--- globals ------------------------------------

static int _ErrorCheck = 0;

//--- prototypes ---------------------------------
// static int	_motor_start_cnt[MOTOR_CNT];
BYTE	_motor_start_cnt[5];
	
static int  _init_done=0;
static int	_motor_end_pos[MOTOR_CNT];
static int  _motor_current_hold[MOTOR_CNT];

//--- motor_get_step ------------------------
INT32	motor_get_step(int motor)
{
	return Fpga.stat->statMot[motor].position;
}

//--- motor_get_end_step ------------------------
INT32	motor_get_end_step(int motor)
{
	return _motor_end_pos[motor];
}


//--- motor_init ---------------------------------
void motor_init(void)
{
	int motor;

	memset(_motor_start_cnt, 0, sizeof(_motor_start_cnt));
	memset(_motor_current_hold, 0, sizeof(_motor_current_hold));
	
	// 4 bit per motor, 0x0e=48V + External  Input + step_clk
	for (motor=0; motor<MOTOR_CNT; motor++)
	{
		Fpga.qsys->pio_sm_out = (Fpga.qsys->pio_sm_out &~ (0x0f<<(4*motor))) | (0x0e<<(4*motor));
	}
	rx_sleep(500);
}

//--- motor_end ---------------------------------
void motor_end(void)
{
	Fpga.qsys->pio_sm_out = 0;
}

//--- motor_reset -----------------------
void motor_reset(int motor)
{
	if (motor <= 4)	// to prevent seg fault !
	{
		Fpga.par->cmd_reset_pos   |= 0x0001<<motor;
		
		_motor_start_cnt[motor] = 0;
	
		Fpga.encoder[motor].pos	   = 0;
		Fpga.encoder[motor].revCnt = 0;
		Fpga.par->cfg[motor].reset_pos_val_enc = 1;
		Fpga.par->cfg[motor].reset_pos_val_mot = 1;
		
		Fpga.par->cfg[motor].stopIn		= 15;
		Fpga.par->cfg[motor].stopLevel  = 0;
		Fpga.par->cfg[motor].disable_mux_in  = 0;		
	}
}

//--- motors_reset ---------------------------------------
void motors_reset(int motors)
{
	int motor;
	FOR_ALL_MOTORS(motor, motors) motor_reset(motor);	
}

//--- motor_main ------------------------------------------
void  motor_main(int ticks, int menu)
{
	/*
	int motor;
	for (motor=0; motor<MOTOR_CNT; motor++)
	{
		if ((Fpga.stat->moving & (0x01<<motor))==0)
			ps_set_current(&Fpga.qsys->spi_powerstep[motor], _motor_current_hold[motor]);
	}
	*/
	if (ps_main())
	{
		Error(ERR_CONT, 0, "Motor Power Supply OFF");
		_init_done=0;
		RX_StepperStatus.info.ref_done = FALSE;
	}
}

//--- motors_init_done -----------------------
int	motors_init_done(void)
{
	return _init_done;			
}

//--- motor_trace_move -----------------------------------------------------
void motor_trace_move(int motor)
{
	SMove	*move;
	int i;
	move = Fpga.move; // starts at index 0

	printf("motor_trace_move: motor=%d\n", motor);
	for (i=0; i<(int)Fpga.par->cfg[motor].moveCnt; i++)
	{
		printf("Move[%d][%d]: cc0=%06d, a=%06d, steps=%06d\n", motor, i, Fpga.move[i].cc0_256, Fpga.move[i].a_256, Fpga.move[i].steps);
	}
	printf("end\n");
	printf("press <ENTER>");
	getchar();
}

//--- motor_move_to_step ---------------------------
int motor_move_to_step(int motor, SMovePar *par, INT32 steps)
{
	return motor_move_by_step(motor, par, steps-motor_get_step(motor));
}

//--- motor_move_by_step ---------------------------------- 
int	motor_move_by_step(int motor, SMovePar *par, INT32 steps)
{
	double		dist;
	int			speed;
	int			accel;
	double		v_max;
	int			bwd;
	SMove		*move;

	// 	t = v/a
	//	d = (a*t^2)/2 = (v^2/a)/2
	//  v = sqrt(2*a/d)

	//--- make sure motor is idle!
	if (abs(steps)<=1) return 0;
	Fpga.par->cfg[motor].stop_mux = par->stop_mux;
	speed=par->speed;
	_motor_start_cnt[motor]++;
	ps_set_current(&Fpga.qsys->spi_powerstep[motor], par->current);
	_motor_end_pos[motor] = Fpga.stat->statMot[motor].position+steps;
	if (steps < 0) 
	{
		speed = -par->speed;
		steps = -steps;
	} 	

	if (speed<0)
	{
		Fpga.par->mot_bwd |= (0x01<<motor);
		ps_set_backwards(&Fpga.qsys->spi_powerstep[motor], FALSE);
		speed=-speed;
	}
	else
	{
		Fpga.par->mot_bwd &= ~(0x01<<motor);
		ps_set_backwards(&Fpga.qsys->spi_powerstep[motor], TRUE);
	}
	
	if (par->estop_in==ESTOP_UNUSED)
	{
		Fpga.par->cfg[motor].estopIn	= 15;
		Fpga.par->cfg[motor].estopLevel = 0;
		Fpga.par->cfg[motor].stopIn		= par->stop_in;  // 15;
		Fpga.par->cfg[motor].stopLevel  = 1 - par->stop_level;  //0;
		Fpga.par->cfg[motor].disable_mux_in  = par->dis_mux_in;
	}
	else
	{
		Fpga.par->cfg[motor].estopIn    = par->estop_in;
		Fpga.par->cfg[motor].estopLevel = 1-par->estop_level;
		Fpga.par->cfg[motor].stopIn		= 15;
		Fpga.par->cfg[motor].stopLevel  = 0;
		Fpga.par->cfg[motor].disable_mux_in  = par->dis_mux_in;
	}
	
	if (par->checkEncoder)
	{
		if (par->sensRef)
		{
			Fpga.par->cfg[motor].enc_rel_steps0 = 10; // 20; // 50; // 100; // 200; // 1; //5;
			Fpga.par->cfg[motor].enc_rel_steps  = 1; // 1; //5;
			Fpga.par->cfg[motor].enc_rel_incs	= 1; // 1; //10;	
		}
		else
		{
			Fpga.par->cfg[motor].enc_rel_steps0 = 200;
			Fpga.par->cfg[motor].enc_rel_steps  = 100;
			Fpga.par->cfg[motor].enc_rel_incs	= 20;	
		}				
	}
	else
	{
		Fpga.par->cfg[motor].enc_rel_steps0 = 0;
		Fpga.par->cfg[motor].enc_rel_steps  = 0;
		Fpga.par->cfg[motor].enc_rel_incs	= 0;								
	}
	move = Fpga.move; // starts at index 0
	move += (motor*MOVE_CNT);

	if (par->accel==0)
	{
		move[0].cc0_256 = speed;
		move[0].a_256   = 0;
		move[0].steps   = steps;
		
		move[1].cc0_256 = 1;
				
		Fpga.par->cfg[motor].moveCnt = 1;
		return 0;
	}

	//dist = speed*speed/par->accel/ 2; // acceleration distance
	dist = (speed*speed-MIN_SPEED_HZ*MIN_SPEED_HZ)/par->accel/ 2; // acceleration distance
	accel= par->accel*FIX_POINT/200000; // [steps/s/5us]
	v_max = sqrt(steps*par->accel + MIN_SPEED_HZ*MIN_SPEED_HZ);
	Fpga.par->cfg[motor].stopAcc_256 = -accel;
	Fpga.par->cfg[motor].stopCC_256  = MIN_SPEED_HZ*FIX_POINT;
	if (steps > 2*dist)
	{
		move[0].cc0_256 = MIN_SPEED_HZ*FIX_POINT;
		move[0].a_256   = accel;
		move[0].steps   = (int)dist;

		move[1].cc0_256 = (UINT32) speed*FIX_POINT ;
		move[1].a_256   = 0;
		move[1].steps   = (int)(steps-2*dist);

		move[2].cc0_256 = (UINT32) speed*FIX_POINT ;
		move[2].a_256   = -accel;
		move[2].steps   = (int)dist;

		move[3].cc0_256 = 1;
		Fpga.par->cfg[motor].moveCnt = 3;
	}
	else
	{
		move[0].cc0_256 = MIN_SPEED_HZ*FIX_POINT;
		move[0].a_256   = accel;
		move[0].steps   = steps/2;

		move[1].cc0_256 = (UINT32)(v_max*FIX_POINT); // (int)(sqrt(steps*par->accel)*FIX_POINT); // [steps/s]
		move[1].a_256   = -accel;
		move[1].steps   = steps-steps/2;

		move[2].cc0_256 = 1;
			
		Fpga.par->cfg[motor].moveCnt = 2;		
	}
	return 1<<motor;
}

//--- motors_start ---------------------------------------
void	motors_start(UINT32 motors, UINT32 errorCheck)
{	
	Fpga.par->cmd_estop |= motors;
	Fpga.par->cmd_start  = motors;
	if (errorCheck) _ErrorCheck |= motors;
	else			_ErrorCheck &= ~motors;
}

//--- motors_move_by_step ---------------------------------------------------------
int motors_move_by_step(int motors, SMovePar *par, INT32 steps, UINT32 errorCheck)
{
	int motor;
	int start=0;
	for (motor=0; motor<MOTOR_CNT; motor++) if (motors & (1<<motor)) start|=motor_move_by_step(motor, par, steps);
	motors_start(start, errorCheck);
	return start;
}

//--- motors_move_to_step ---------------------------------------------------------
int    motors_move_to_step(int motors, SMovePar *par, INT32 steps)
{
	int motor;
	int start=0;
	for (motor=0; motor<MOTOR_CNT; motor++) if (motors & (1<<motor)) start|=motor_move_to_step(motor, par, steps);
	motors_start(start, TRUE);
	return start;
}

//--- motors_quad_move_by_step ---------------------------------------------------------
int motors_quad_move_by_step(int motors, SMovePar *par[4], INT32 steps, UINT32 errorCheck)
{
	int motor;
	int start=0;
	for (motor = 0; motor < MOTOR_CNT; motor++) if (motors & (1 << motor)) start|=motor_move_by_step(motor, par[motor], steps);
	motors_start(start, errorCheck);
	return start;
}

//--- motors_stop ---------------------------------------
void motors_stop (UINT32 motors)
{
	Fpga.par->cmd_stop  = motors;
}

//--- motors_estop ---------------------------------------
void motors_estop(UINT32 motors)
{
	Fpga.par->cmd_estop  &= !motors; // TODO: check
}

//--- motor_move_done -------------------------------------
int	motor_move_done(int motor)
{
	if (!fpga_is_init()) return FALSE;
	BYTE cnt=Fpga.stat->statMot[motor].moving_cnt;
	if (cnt == _motor_start_cnt[motor]) // _motor_start_cnt[motor]>0 &&
	{
		ps_set_current(&Fpga.qsys->spi_powerstep[motor], _motor_current_hold[motor]);
		return TRUE;
	}
	return FALSE;
}

//--- motor_set_hold_current -------------------------------------
void	motor_set_hold_current(int motor)
{
	ps_set_current(&Fpga.qsys->spi_powerstep[motor], _motor_current_hold[motor]);
}

//--- motors_move_done -----------------------------------------
int	motors_move_done(int motors)
{
	int motor;
	FOR_ALL_MOTORS(motor, motors)
	{
		if (!motor_move_done(motor)) return FALSE;
	}
	return TRUE;
}

//--- motor_error ----------------------------------------
int	motor_error(int motor)
{
	if ((Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_ENC) && (_ErrorCheck & (1<<motor))) return TRUE;
	return FALSE;
}

//--- motors_error -----------------------------
int	motors_error(int motors, int *err)
{
	int motor;
	*err=0;
	FOR_ALL_MOTORS(motor, motors)
	{
		if (motor_error(motor)) {*err=motor; return TRUE;};			
	}
	return FALSE;
}


//--- motor_config ---------------------
void motor_config(int motor, int currentHold, double stepsPerMeter, double incPerMeter)
{
	if (ps_get_power()<20000) return;
	if (_init_done & (0x01<<motor)) return;
	_init_done |= (0x01<<motor);

	ps_init(&Fpga.qsys->spi_powerstep[motor], NULL);

	_motor_current_hold[motor] = currentHold;
	ps_set_current(&Fpga.qsys->spi_powerstep[motor], 0);	// set 0 until first move done
	
	Fpga.par->cmd_estop |= (0x01<<motor);
	Fpga.par->cfg[motor].enc_bwd = FALSE;
	Fpga.par->cfg[motor].enc_stop_index = FALSE;
	
	Fpga.par->cfg[motor].enc_stall_en	= FALSE;
	Fpga.par->cfg[motor].enc_stall_var  = 200;
	Fpga.par->cfg[motor].enc_mot_ratio	= (round((incPerMeter / stepsPerMeter) * (2 ^ 24))); // mot/enc=ratio
	
	Fpga.par->cfg[motor].enc_rel_steps0 = 200;
	Fpga.par->cfg[motor].enc_rel_steps  = 100;
	Fpga.par->cfg[motor].enc_rel_incs	= 20;		

	Fpga.par->cfg[motor].stop_mux = 0x00; // stop motors together
	
//	Fpga.par->cfg[motor].estopIn	= 15;
//	Fpga.par->cfg[motor].estopLevel = 0;
	Fpga.par->cfg[motor].stopIn		= 15;
	Fpga.par->cfg[motor].stopLevel  = 0;	
	Fpga.par->cfg[motor].disable_mux_in  = 0;
}

//--- motors_config -----------------------------------------
void motors_config(int motors, int currentHold, double stepsPerMeter, double incPerMeter)
{
	int motor;
	FOR_ALL_MOTORS(motor, motors)
	{
		motor_config(motor, currentHold, stepsPerMeter, incPerMeter);
	}
}

// --- motors_dual_move_to_step(MOTOR_Z_BITS, &_ParZ_down, -pos)
int motors_dual_move_to_step(int motors, SMovePar *par_pos, SMovePar *par_neg, INT32 steps)
{
	int motor;
	int motor_cnt = 0;
	int	motor_neg = 0;
	
	FOR_ALL_MOTORS(motor, motors)
	{
		motor_cnt++;
		if (steps < motor_get_step(motor))
		{
			motor_neg++;
		}
	}
	
	if ((motor_cnt / 2.0) < motor_neg)	motors_move_to_step(motors, par_neg, steps);
	else								motors_move_to_step(motors, par_pos, steps);
	return motors;
}
