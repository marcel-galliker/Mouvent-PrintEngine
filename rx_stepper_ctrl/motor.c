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

#define MIN_SPEED_HZ				100		// minimal speed in Hz
#define ENC_STOP_MIN_SPEED_HZ		2500
#define MAX_SPEED_HZ				17000		// maximal speed in Hz

#define FIX_POINT		0x10000

//--- globals ------------------------------------

static int _ErrorCheck = 0;

//--- prototypes ---------------------------------
BYTE	_motor_start_cnt[MOTOR_CNT];
	
static int		_init_done = 0;
static int		_motor_end_pos[MOTOR_CNT];
static int		_current_hold[MOTOR_CNT];
static double	_enc_mot_ratio[MOTOR_CNT];
static int		_message_written = FALSE;

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
	memset(_enc_mot_ratio, 0, sizeof(_enc_mot_ratio));
	
	// 4 bit per motor, 0x0e=48V + External  Input + step_clk
	for (motor = 0; motor < MOTOR_CNT; motor++)
	{
		Fpga.qsys->pio_sm_out = (Fpga.qsys->pio_sm_out & ~(0x0f << (4*motor))) | (0x0e << (4*motor));
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
		Fpga.par->cmd_reset_pos   |= 0x0001 << motor;
		
		_motor_start_cnt[motor] = 0;
	
		Fpga.encoder[motor].pos	   = 0;
		Fpga.encoder[motor].revCnt = 0;
		Fpga.par->cfg[motor].reset_pos_val_enc = 1;
		Fpga.par->cfg[motor].reset_pos_val_mot = 1;
		
//		Fpga.par->cfg[motor].stopIn		= 15;
//		Fpga.par->cfg[motor].stopLevel  = 0;
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
	if (ps_main())
	{
		if (_message_written == FALSE)	Error(ERR_CONT, 0, "Motor Power Supply OFF");
		_init_done = 0;
		_message_written = TRUE;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.robinfo.ref_done = FALSE;
	}
	else	_message_written = FALSE;
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
	move = Fpga.move;

	printf("motor_trace_move: motor=%d\n", motor);
	for (i = 0; i < (int)Fpga.par->cfg[motor].moveCnt; i++)
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
	return motor_move_by_step(motor, par, steps - motor_get_step(motor));
}

//--- motor_move_by_step ---------------------------------- 
int	motor_move_by_step(int motor, SMovePar *par, INT32 steps)
{
#define CURRENT_OFFSET	1000
	double		dist_rising_ramp;
	double		dist_falling_ramp;
	double		dist_ramp_total;
	int			speed;
	int			accel;
	int			cnt;
	double		v_max;
	int			bwd;
	SMove		*move;

	// 	t = v/a
	//	d = (a*t^2)/2 = (v^2/a)/2
	//  v = sqrt(2*a/d)

	//--- make sure motor is idle!
	if (abs(steps) <= 1) return 0;

	_motor_start_cnt[motor]++;
	
	Fpga.par->cfg[motor].estopIn			= par->estop_in_bit[motor];
	Fpga.par->cfg[motor].estopLevel			= (par->estop_in_bit[motor])? 1 - par->estop_level : 0;
	Fpga.par->cfg[motor].unused_stopIn		= 0;
	Fpga.par->cfg[motor].unused_stopLevel	= 0;
	Fpga.par->cfg[motor].stop_mux			= par->stop_mux;
	Fpga.par->cfg[motor].disable_mux_in		= par->dis_mux_in;
	speed = par->speed;
	
	// Set target pos
	_motor_end_pos[motor] = Fpga.stat->statMot[motor].position + steps;
	
	if (steps < 0) 
	{
		speed = -par->speed;
		steps = -steps;
	} 	

	if (speed < 0)
	{
		Fpga.par->mot_bwd |= (0x01 << motor);
		ps_set_backwards(&Fpga.qsys->spi_powerstep[motor], FALSE);
		speed = -speed;
	}
	else
	{
		Fpga.par->mot_bwd &= ~(0x01 << motor);
		ps_set_backwards(&Fpga.qsys->spi_powerstep[motor], TRUE);
	}
		
	switch(par->encCheck)
	{	
	case chk_std:
		Fpga.par->cfg[motor].enc_rel_steps0 = 200;
		Fpga.par->cfg[motor].enc_rel_steps  = 100;
		Fpga.par->cfg[motor].enc_rel_incs	= 20;	
		break;
		
	case chk_txrob_ref:
		Fpga.par->cfg[motor].enc_rel_steps0 = 10;
		Fpga.par->cfg[motor].enc_rel_steps  = 1;
		Fpga.par->cfg[motor].enc_rel_incs	= 1;
		break;

	case chk_txrob:
		Fpga.par->cfg[motor].enc_rel_steps0 = 10;
		Fpga.par->cfg[motor].enc_rel_steps  = 2;
		Fpga.par->cfg[motor].enc_rel_incs	= 1;	
		break;

	case chk_lbrob:
		Fpga.par->cfg[motor].enc_rel_steps0 = 2000;
		Fpga.par->cfg[motor].enc_rel_steps  = 1000;
		Fpga.par->cfg[motor].enc_rel_incs	= 20;	
		break;

	case chk_off:
	default:
		Fpga.par->cfg[motor].enc_rel_steps0 = 0;
		Fpga.par->cfg[motor].enc_rel_steps  = 0;
		Fpga.par->cfg[motor].enc_rel_incs	= 0;								
		break;
	}
	
	move = Fpga.move; // starts at index 0
	move += (motor*MOVE_CNT);
	
	Fpga.par->cfg[motor].enc_bwd = par->enc_bwd;
	
	if (par->accel == 0)
		return 0;
	
	Fpga.par->cfg[motor].amp_stop	 = Tval_Current_to_Par(par->current_acc);
	Fpga.par->cfg[motor].v_min_speed = MIN_SPEED_HZ*FIX_POINT;
	
	// Calculate ramps	
	// Rising ramp is always the same
	dist_rising_ramp = (speed*speed - MIN_SPEED_HZ*MIN_SPEED_HZ) / par->accel / 2;
	
	if (par->enc_mode)
		dist_falling_ramp = (speed*speed - ENC_STOP_MIN_SPEED_HZ*ENC_STOP_MIN_SPEED_HZ) / par->accel / 2;
	else
		dist_falling_ramp = dist_rising_ramp;
	
	dist_ramp_total = dist_rising_ramp + dist_falling_ramp;
	
	accel = par->accel*FIX_POINT / 200000; // [steps/s/5us]
	v_max = sqrt(steps*par->accel + MIN_SPEED_HZ*MIN_SPEED_HZ);
	Fpga.par->cfg[motor].stopAcc_256 = -accel;
	Fpga.par->cfg[motor].stopCC_256  = MIN_SPEED_HZ*FIX_POINT;
	
	// If the total distance is longer than the length of both ramps
	if (steps > dist_ramp_total)
	{
		int rising_steps;
		int linear_steps;
		int falling_steps;
		int min_speed;
		
		rising_steps = (int)dist_rising_ramp;
		linear_steps = (int)(steps - dist_ramp_total);
		falling_steps = (int)dist_falling_ramp;
		
		// Dividing by encoder step to morot step ratio if we drive to a target position based on the encoder
		if(par->enc_mode)
		{
			rising_steps  /= _enc_mot_ratio[motor];
			linear_steps  /= _enc_mot_ratio[motor];
			falling_steps /= _enc_mot_ratio[motor];
			min_speed = ENC_STOP_MIN_SPEED_HZ*FIX_POINT;
		}
		else
		{
			min_speed = MIN_SPEED_HZ*FIX_POINT;
		}
		
		cnt=0;
		// Set rising ramp parameters
		move[cnt].cc0_256 = (UINT32) speed*FIX_POINT;
		move[cnt].a_256   = accel;
		move[cnt].steps   = rising_steps;
		move[cnt].amp	  = Tval_Current_to_Par(par->current_acc);
		cnt++;
		
		// Set linear movement parameters 
		if (linear_steps>CURRENT_OFFSET)
		{
			move[cnt].cc0_256 = (UINT32) speed*FIX_POINT;
			move[cnt].a_256   = 0;
			move[cnt].steps   = linear_steps-CURRENT_OFFSET;
			move[cnt].amp	  = Tval_Current_to_Par(par->current_run);			
			cnt++;
			
			move[cnt].cc0_256 = (UINT32) speed*FIX_POINT;
			move[cnt].a_256   = 0;
			move[cnt].steps   = CURRENT_OFFSET;
			move[cnt].amp	  = Tval_Current_to_Par(par->current_acc);
			cnt++;
		}
		else
		{
			move[cnt].cc0_256 = (UINT32) speed*FIX_POINT;
			move[cnt].a_256   = 0;
			move[cnt].steps   = linear_steps;
			move[cnt].amp	  = Tval_Current_to_Par(par->current_acc);
			cnt++;
		}
		
		// Set falling ramp parameters
		move[cnt].cc0_256 = min_speed;
		move[cnt].a_256   = -accel;
		move[cnt].steps   = falling_steps;
		move[cnt].amp	  = Tval_Current_to_Par(par->current_acc);
		cnt++;

		// Set holding current after move done
		move[cnt].cc0_256 = 1;
		move[cnt].a_256   = 1;
		move[cnt].steps	  = 0;
		move[cnt].amp	  = Tval_Current_to_Par(_current_hold[motor]);
		cnt++;
		
	//	int steps_total = move[0].steps + move[1].steps + move[2].steps;
		
		Fpga.par->cfg[motor].moveCnt = cnt;
	}
	else
	{
		// If the total distance is shorter than the sum of both ramps we need to load the registers differently
		int rising_steps;
		int falling_steps;
		int min_speed;

		// If we drive by encoder steps, we need to 
		if (par->enc_mode)
		{
			rising_steps  = ((steps*dist_rising_ramp) / (dist_rising_ramp + dist_falling_ramp)) / _enc_mot_ratio[motor];
			falling_steps = ((steps*dist_falling_ramp) / (dist_rising_ramp + dist_falling_ramp)) / _enc_mot_ratio[motor];
			min_speed = ENC_STOP_MIN_SPEED_HZ*FIX_POINT;
		}
		else
		{
			rising_steps  = (int)(steps / 2);
			falling_steps = (int)(steps - (steps / 2));
			min_speed = MIN_SPEED_HZ*FIX_POINT;
		}
		
		// Set rising ramp parameters
		move[0].cc0_256 = (UINT32)(v_max*FIX_POINT);
		move[0].a_256   = accel;
		move[0].steps   = rising_steps;
		move[0].amp		= Tval_Current_to_Par(par->current_acc);

		// Set falling ramp parameters
		move[1].cc0_256 = min_speed;
		move[1].a_256   = -accel;
		move[1].steps   = falling_steps;
		move[1].amp		= Tval_Current_to_Par(par->current_acc);

		// Set holding current after move done
		move[2].cc0_256 = 1;
		move[2].a_256   = 1;
		move[2].steps	= 0;
		move[2].amp		= Tval_Current_to_Par(_current_hold[motor]);
			
		Fpga.par->cfg[motor].moveCnt = 3;		
	}
	return 1 << motor;
}

//--- motors_start ---------------------------------------
void motors_start(UINT32 motors, UINT32 errorCheck)
{	
	Fpga.par->cmd_estop |= motors;
	Fpga.par->cmd_start  = motors;
	if (errorCheck) _ErrorCheck |= motors;
	else			_ErrorCheck &= ~motors;
}

//--- motors_start ---------------------------------------
void motors_start_enc_reg(UINT32 motors, UINT32 errorCheck)
{	
	Fpga.par->cmd_estop |= motors;
	Fpga.par->cmd_start_encmot  = motors;
	if (errorCheck) _ErrorCheck |= motors;
	else			_ErrorCheck &= ~motors;
}

//--- motors_move_by_step ---------------------------------------------------------
int motors_move_by_step(int motors, SMovePar *par, INT32 steps, UINT32 errorCheck)
{
	int motor;
	int start = 0;
	for (motor = 0; motor < MOTOR_CNT; motor++) if (motors & (1 << motor)) start |= motor_move_by_step(motor, par, steps);
	motors_start(start, errorCheck);
	return start;
}

//--- motors_move_to_step ---------------------------------------------------------
int    motors_move_to_step(int motors, SMovePar *par, INT32 steps)
{
	int motor;
	int start = 0;
	for (motor = 0; motor < MOTOR_CNT; motor++) if (motors & (1 << motor)) start |= motor_move_to_step(motor, par, steps);
	motors_start(start, TRUE);
	return start;
}

//--- motors_quad_move_by_step ---------------------------------------------------------
int motors_quad_move_by_step(int motors, SMovePar *par[4], INT32 steps, UINT32 errorCheck)
{
	int motor;
	int start = 0;
	for (motor = 0; motor < MOTOR_CNT; motor++) if (motors & (1 << motor)) start |= motor_move_by_step(motor, par[motor], steps);
	motors_start(start, errorCheck);
	return start;
}

//--- motors_stop ---------------------------------------
void motors_stop(UINT32 motors)
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
	return (Fpga.stat->statMot[motor].moving_cnt == _motor_start_cnt[motor]);
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
	if ((Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_ENC) && !(Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_EMUX) && (_ErrorCheck & (1 << motor))) return TRUE;
	return FALSE;
}

//--- motors_error -----------------------------
int	motors_error(int motors, int *err)
{
	int motor;
	*err = 0;
	FOR_ALL_MOTORS(motor, motors)
	{
		if (motor_error(motor)) 
		{
			*err = motor; 
			return TRUE;
		}
	}
	return FALSE;
}


//--- motor_config ---------------------
void motor_config(int motor, int currentHold, double stepsPerMeter, double incPerMeter)
{
	if (ps_get_power() < 20000) return;
	if (_init_done & (0x01 << motor)) return;
	_init_done |= (0x01 << motor);

	ps_init(&Fpga.qsys->spi_powerstep[motor], NULL);

	_current_hold[motor] = currentHold;
	Fpga.par->cfg[motor].amp_idle = Tval_Current_to_Par(currentHold);
	
	Fpga.par->cmd_estop |= (0x01 << motor);
	Fpga.par->cfg[motor].enc_bwd = FALSE;
	Fpga.par->cfg[motor].enc_stop_index = FALSE;
	
	_enc_mot_ratio[motor] = (double)((double)stepsPerMeter / (double)incPerMeter); // mot/enc=ratio
	
	Fpga.par->cfg[motor].enc_rel_steps0 = 200;
	Fpga.par->cfg[motor].enc_rel_steps  = 100;
	Fpga.par->cfg[motor].enc_rel_incs	= 20;
	Fpga.par->cfg[motor].stop_mux = 0x00; // stop motors together
	
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
