// ****************************************************************************
//
//	DIGITAL PRINTING - lb701.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_error.h"
#include "rx_term.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "lb701.h"

#define MOTOR_Z_0		0
#define MOTOR_Z_BITS	0x01

#define CURRENT_HOLD	50

#define HEAD_UP_IN		0
#define POS_UP			1000

#define STEPS_REV		200*16	// steps per motor revolution * 16 times oversampling
#define DIST_REV		2000.0	// moving distance per revolution [µm]

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_cap;
static int		_CmdRunning=0;
static char		_CmdName[32];
static int		_PrintPos_New=0;
static int		_PrintPos_Act=0;


//--- prototypes --------------------------------------------
static void _lb701_motor_z_test(int steps);
static void _lb701_motor_test  (int motor, int steps);
static void _lb701_do_reference(void);
static void _lb701_move_to_pos(int cmd, int pos);
static int  _micron_2_steps(int micron);

//--- lb701_init --------------------------------------
void lb701_init(void)
{
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);
	memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
	_ParRef.speed		= 10000;
	_ParRef.accel		= 5000;
	_ParRef.current		= 150.0;
	_ParRef.stop_mux	= 0;
	_ParRef.dis_mux_in	= 0;
	_ParRef.stop_in		= ESTOP_UNUSED;
	_ParRef.stop_level	= 0;
	_ParRef.estop_in    = HEAD_UP_IN;
	_ParRef.estop_level = 1;
	_ParRef.checkEncoder=TRUE;
	
	_ParZ_down.speed		= 40000;
	_ParZ_down.accel		= 10000;
	_ParZ_down.current		= 300.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.stop_in		= ESTOP_UNUSED;
	_ParZ_down.stop_level	= 0;
	_ParZ_down.estop_in     = ESTOP_UNUSED;
	_ParZ_down.estop_level  = 0;
	_ParZ_down.checkEncoder = TRUE;

	_ParZ_cap.speed			= 1000;
	_ParZ_cap.accel			= 1000;
	_ParZ_cap.current		= 100.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= 0;
	_ParZ_cap.stop_in       = ESTOP_UNUSED;
	_ParZ_cap.stop_level    = 0;
	_ParZ_cap.estop_in      = ESTOP_UNUSED;
	_ParZ_cap.estop_level   = 0;
	_ParZ_cap.checkEncoder  = TRUE;
}

//--- lb701_main ------------------------------------------------------------------
void lb701_main(int ticks, int menu)
{
	int motor;
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN);
	RX_StepperStatus.posZ				  = motor_get_step(MOTOR_Z_0);

	if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;			
	}
	if (_CmdRunning==0) RX_StepperStatus.info.moving = FALSE;
	if (_CmdRunning && motors_move_done(MOTOR_Z_BITS)) 
	{
		RX_StepperStatus.info.moving = FALSE;
		if (_CmdRunning == CMD_CAP_REFERENCE) 
		{
			if (motor_error(MOTOR_Z_BITS))
			{	
				Error(ERR_CONT, 0, "LB701: Command CMD_CAP_REFERENCE: triggers motor_error", _CmdRunning);
				RX_StepperStatus.info.ref_done = FALSE;
			}
			else
			{
				motors_reset(MOTOR_Z_BITS);				
				RX_StepperStatus.info.ref_done = TRUE;
			}
		}
		else if (motors_error(MOTOR_Z_BITS, &motor))
		{
			Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			RX_StepperStatus.info.ref_done = FALSE;							
		}
		RX_StepperStatus.info.z_in_ref    = ((_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS) && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_print  = (_CmdRunning==CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_cap    = (_CmdRunning==CMD_CAP_CAPPING_POS);
		if (RX_StepperStatus.info.ref_done && _CmdRunning == CMD_CAP_REFERENCE && _PrintPos_New) 
		{
			if (_PrintPos_New==POS_UP) _lb701_move_to_pos(CMD_CAP_UP_POS,    _PrintPos_New);
			else                       _lb701_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
			_PrintPos_Act = _PrintPos_New;
			_PrintPos_New = 0;
		}
		else {
		//	RX_StepperStatus.info.move_tgl = !RX_StepperStatus.info.move_tgl;
			_CmdRunning = FALSE;
		}
	}	
}

//--- _lb701_display_status ---------------------------------------------------------
static void _lb701_display_status(void)
{
	term_printf("LB 701 ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n",	RX_StepperStatus.info.moving, _CmdRunning);	
	term_printf("Head UP Sensor: %d\n",	fpga_input(HEAD_UP_IN));	
	term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
	term_printf("z in reference: %d\n",	RX_StepperStatus.info.z_in_ref);
	term_printf("z in print:     %d\n",	RX_StepperStatus.info.z_in_print);
	term_printf("z in capping:   %d\n",	RX_StepperStatus.info.z_in_cap);
	term_printf("\n");
}

//--- lb701_menu --------------------------------------------------
int lb701_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int pos=10000;

	_lb701_display_status();
	
	term_printf("LB 701 MENU -------------------------\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("R: Reference\n");
	term_printf("c: move to cap\n");
	term_printf("u: move to UP position\n");
	term_printf("p: move to print\n");
	term_printf("z: move by <steps>\n");
	term_printf("o<n>: toggle output <n>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': lb701_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,			NULL); break;
		case 'R': lb701_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,		NULL); break;
		case 'r': motor_reset(atoi(&str[1]));										   break;
		case 'c': lb701_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS,	NULL); break;
		case 'p': lb701_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,		&pos); break;
		case 'u': lb701_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS,			NULL); break;
		case 'o': fpga_output_toggle(atoi(&str[1]));break;
		case 'z': _lb701_motor_z_test(atoi(&str[1]));break;
		case 'm': _lb701_motor_test(str[1]-'0', atoi(&str[2]));break;			
		case 'x': return FALSE;
		}
	}
	return TRUE;
}


//--- _lb701_do_reference ----------------------------------------------------------------
static void _lb701_do_reference(void)
{
	motors_stop	(MOTOR_Z_BITS);
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);
	
	_CmdRunning  = CMD_CAP_REFERENCE;
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, -500000, TRUE);
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5+STEPS_REV/DIST_REV*micron);
}

//--- _lb701_move_to_pos ---------------------------------------------------------------
static void _lb701_move_to_pos(int cmd, int pos)
{
	_CmdRunning  = cmd;
	RX_StepperStatus.info.moving = TRUE;
	motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
}

//--- lb701_handle_ctrl_msg -----------------------------------
int  lb701_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	INT32 pos, steps;
	
	switch(msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	
									break;

	case CMD_CAP_STOP:				strcpy(_CmdName, "CMD_CAP_STOP");
									motors_stop(MOTOR_Z_BITS);
									_CmdRunning = 0;
									break;	

	case CMD_CAP_REFERENCE:			strcpy(_CmdName, "CMD_CAP_REFERENCE");
									_PrintPos_New=0;
									_lb701_do_reference();	
									break;

	case CMD_CAP_PRINT_POS:			strcpy(_CmdName, "CMD_CAP_PRINT_POS");
									pos   = (*((INT32*)pdata));
									steps = _micron_2_steps(RX_StepperCfg.ref_height - pos);
									if (!_CmdRunning && (!RX_StepperStatus.info.z_in_print || steps!=_PrintPos_Act))
									{
										_PrintPos_New = _micron_2_steps(RX_StepperCfg.ref_height - pos);
										if (RX_StepperStatus.info.ref_done) _lb701_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
										else								_lb701_do_reference();
									}
									break;
		
	case CMD_CAP_UP_POS:			strcpy(_CmdName, "CMD_CAP_UP_POS");
									if (!_CmdRunning)
									{
										if (RX_StepperStatus.info.ref_done) _lb701_move_to_pos(CMD_CAP_UP_POS, POS_UP);
										else								_lb701_do_reference();
									}
									break;

	case CMD_CAP_CAPPING_POS:		strcpy(_CmdName, "CMD_CAP_CAPPING_POS");
									if (!_CmdRunning)
									{
										_CmdRunning  = msgId;
										RX_StepperStatus.info.moving = TRUE;
										motors_move_to_step	(MOTOR_Z_BITS,  &_ParZ_cap, _micron_2_steps(RX_StepperCfg.cap_height));
									}
									break;
		
	case CMD_ERROR_RESET:			strcpy(_CmdName, "CMD_ERROR_RESET");
									fpga_stepper_error_reset();
									break;
		
	case CMD_CAP_VENT:	break;
		
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _lb701_motor_z_test ----------------------------------------------------------------------
void _lb701_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, steps, TRUE);
}

//--- _lb701_motor_test ---------------------------------
static void _lb701_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;
	int i;

	par.speed		= 10000;
	par.accel		= 5000;
	par.current		= 250.0;
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level  = 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	
	motors_config(motors, CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motors, &par, steps, FALSE);			
}

