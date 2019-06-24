// ****************************************************************************
//
//	DIGITAL PRINTING - dp803.c
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
#include "rx_trace.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "dp803.h"

#define MOTOR_Z_0		0
#define MOTOR_Z_BITS	0x0f

#define CURRENT_HOLD	50

#define HEAD_UP_IN_0	1
#define HEAD_UP_IN_1	0
#define POS_UP			-1000

#define STEPS_REV		200*16	// steps per motor revolution * 16 times oversampling
#define DIST_REV		2000.0	// moving distance per revolution [µm]

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_cap;
static int		_CmdRunning=0;
static char		_CmdName[32];
static int		_PrintPos_New=0;
static int		_PrintPos_Act=0;
static int		_PrintHeight=0;


//--- prototypes --------------------------------------------
static void _dp803_motor_z_test(int steps);
static void _dp803_motor_test  (int motor, int steps);
static void _dp803_do_reference(void);
static void _dp803_move_to_pos(int cmd, int pos);
static int  _micron_2_steps(int micron);
static void reinit(void);

//--- dp803_init --------------------------------------
void dp803_init(void)
{
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);
	memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
	_ParRef.speed			= 10000;
	_ParRef.accel			= 5000;
//	_ParRef.current			= 150.0;
	_ParRef.current			= 250.0;
	_ParRef.stop_mux		= 0;
	_ParRef.dis_mux_in		= 0;
	_ParRef.stop_in			= ESTOP_UNUSED;
	_ParRef.stop_level		= 0;
	_ParRef.estop_in		= ESTOP_UNUSED;
	_ParRef.estop_level		= 0;
	_ParRef.checkEncoder	=TRUE;
	
	_ParZ_down.speed		= 10000;
	_ParZ_down.accel		= 5000;
	_ParZ_down.current		= 400.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.stop_in      = ESTOP_UNUSED;
	_ParZ_down.stop_level   = 0;
	_ParZ_down.estop_in     = ESTOP_UNUSED;
	_ParZ_down.estop_level  = 0;
	_ParZ_down.checkEncoder = TRUE;

	_ParZ_cap.speed			= 1000;
	_ParZ_cap.accel			= 1000;
	_ParZ_cap.current		= 100.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= 0;
	_ParZ_cap.stop_in		= ESTOP_UNUSED;
	_ParZ_cap.stop_level	= 0;
	_ParZ_cap.estop_in      = ESTOP_UNUSED;
	_ParZ_cap.estop_level   = 0;
	_ParZ_cap.checkEncoder  = TRUE;
}

//--- dp803_main ------------------------------------------------------------------
void dp803_main(int ticks, int menu)
{
	int motor;
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
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
			if (!RX_StepperStatus.info.headUpInput_0) Error(ERR_CONT, 0, "dp803: Command REFERENCE: End Sensor 1 NOT HIGH");
			if (!RX_StepperStatus.info.headUpInput_1) Error(ERR_CONT, 0, "dp803: Command REFERENCE: End Sensor 2 NOT HIGH");
			RX_StepperStatus.info.ref_done =  RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
			motors_reset(MOTOR_Z_BITS);				
		}
		else if (motors_error(MOTOR_Z_BITS, &motor))
		{
			Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			RX_StepperStatus.info.ref_done = FALSE;							
		}
		RX_StepperStatus.info.z_in_ref    = ((_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS) && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_print  = (_CmdRunning==CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_cap    = (_CmdRunning==CMD_CAP_CAPPING_POS);
		if (_CmdRunning == CMD_CAP_REFERENCE && _PrintPos_New) 
		{
			_dp803_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
			_PrintPos_Act = _PrintPos_New;
			_PrintPos_New = 0;
		}
		else {
			RX_StepperStatus.info.move_tgl = !RX_StepperStatus.info.move_tgl;
			_CmdRunning = FALSE;
		}
	}	
}

//--- _dp803_display_status ---------------------------------------------------------
static void _dp803_display_status(void)
{
	term_printf("DP 803 ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n",	RX_StepperStatus.info.moving, _CmdRunning);
	term_printf("actpos:         %06d  newpos: %06d\n",	_PrintPos_Act, _PrintPos_New);		
	term_printf("refheight:      %06d  ph:     %06d\n", 	_micron_2_steps(RX_StepperCfg.ref_height), _micron_2_steps(_PrintHeight));
	term_printf("Head UP Sensor: %d  %d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1));	
	term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
	term_printf("z in reference: %d\n",	RX_StepperStatus.info.z_in_ref);
	term_printf("z in print:     %d\n",	RX_StepperStatus.info.z_in_print);
	term_printf("z in capping:   %d\n",	RX_StepperStatus.info.z_in_cap);
	term_printf("\n");
}

//--- dp803_menu --------------------------------------------------
int dp803_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int pos=10000;

	_dp803_display_status();
	
	term_printf("DP 803 MENU -------------------------\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("R: Reference\n");
	term_printf("c: move to cap\n");
	term_printf("u: move to UP position\n");
	term_printf("p: move to print\n");
	term_printf("z: move by <steps>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': dp803_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,		NULL); break;
		case 'R': dp803_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,	NULL); break;
		case 'r': motor_reset(atoi(&str[1])); break;
		case 'c': dp803_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS,NULL); break;
		case 'p': dp803_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,	&pos); break;
		case 'u': dp803_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS,		NULL); break;
		case 'z': _dp803_motor_z_test(atoi(&str[1]));break;
		case 'm': _dp803_motor_test(str[1]-'0', atoi(&str[2]));break;
		case 'o': Fpga.par->adc_rst = FALSE; break;
		case 'i': Fpga.par->adc_rst = TRUE; break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}

static void reinit(void)
{
	
	
	fpga_init();

}

//--- _dp803_do_reference ----------------------------------------------------------------
static void _dp803_do_reference(void)
{
	TrPrintfL(TRUE, "_dp803_do_reference");
	Error(LOG, 0, "_dp803_do_reference");
	motors_stop	(MOTOR_Z_BITS);
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);
	_CmdRunning  = CMD_CAP_REFERENCE;
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, 500000, TRUE);
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5+STEPS_REV/DIST_REV*micron);
}

//--- _dp803_move_to_pos ---------------------------------------------------------------
static void _dp803_move_to_pos(int cmd, int pos)
{
	_CmdRunning  = cmd;
	RX_StepperStatus.info.moving = TRUE;
	motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
}

//--- dp803_handle_ctrl_msg -----------------------------------
int  dp803_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
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

	case CMD_CAP_REFERENCE:			_PrintPos_New=0;
									strcpy(_CmdName, "CMD_CAP_REFERENCE");
									_dp803_do_reference();	
									break;

	case CMD_CAP_PRINT_POS:			strcpy(_CmdName, "CMD_CAP_PRINT_POS");
									pos   = (*((INT32*)pdata));
									_PrintHeight = pos;
									steps = _micron_2_steps(RX_StepperCfg.ref_height - pos);
									if (!_CmdRunning && (!RX_StepperStatus.info.z_in_print || steps!=_PrintPos_Act))
									{
										_PrintPos_New = -1*_micron_2_steps(RX_StepperCfg.ref_height - pos);
										if (RX_StepperStatus.info.ref_done) _dp803_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
										else								  _dp803_do_reference();
									}
									break;
		
	case CMD_CAP_UP_POS:			strcpy(_CmdName, "CMD_CAP_UP_POS");
									if (!_CmdRunning)
									{
										if (RX_StepperStatus.info.ref_done) _dp803_move_to_pos(CMD_CAP_UP_POS, -1*_micron_2_steps(RX_StepperCfg.ref_height - 20000));
										else								  _dp803_do_reference();
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

//--- _dp803_motor_z_test ----------------------------------------------------------------------
void _dp803_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, steps, TRUE);
}

//--- _dp803_motor_test ---------------------------------
static void _dp803_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;
	int i;

	par.speed		= 5000;
	par.accel		= 2500;
	par.current		= 400.0;
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	
	motors_config(motors, CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motors, &par, steps, FALSE);			
}