// ****************************************************************************
//
//	DIGITAL PRINTING - tx801.c
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
#include "stepper_ctrl.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "tx801.h"

#define TX_REF_HEIGHT		16000
#define TX_PRINT_POS_MIN	 1200

#define MOTOR_Z_0		0
#define MOTOR_Z_CNT		4
#define MOTOR_Z_BITS	0x0f

#define CURRENT_HOLD	50

#define HEAD_UP_IN_0		0
#define HEAD_UP_IN_1		1
#define HEAD_UP_IN_2		2
#define HEAD_UP_IN_3		3

#define STEPS_REV		(200*16)	// steps per motor revolution * 16 times oversampling
#define INCS_REV		4000		// encoder increments per revolution
#define DIST_REV		1000.0		// moving distance per revolution [µm]

#define POS_UP			(STEPS_REV/DIST_REV*1000)

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_cap;
static int		_CmdRunning=0;
static int		_PrintPos_New=0;
static int		_PrintPos_Act=0;

//--- prototypes --------------------------------------------
static void _tx801_motor_z_test(int steps);
static void _tx801_motor_test  (int motor, int steps);
static void _tx801_do_reference(void);
static void _tx801_move_to_pos(int cmd, int pos);
static void _tx801_set_ventilators(int value);
static int  _micron_2_steps(int micron);// microns to motor steps
static int  _steps_2_micron(int steps); // motor steps to microns
static int  _incs_2_micron(int steps);  // encoder_incs to microns
static char	*_motor_name(int no);

//--- tx801_init --------------------------------------
void tx801_init(void)
{
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);
	
	//--- movment parameters ----------------
	/*
	_ParRef.speed		= 2000;
	_ParRef.accel		= 1000;
	_ParRef.current		= 100.0;
	_ParRef.stop_mux	= 0;
	_ParRef.dis_mux_in	= 0;
	_ParRef.stop_in		= ESTOP_UNUSED;
	_ParRef.stop_level	= 0;
	_ParRef.estop_in    = ESTOP_UNUSED;
	_ParRef.estop_level = 0;
	_ParRef.checkEncoder=TRUE;
	*/
	_ParRef.speed		= 10000;
	_ParRef.accel		= 5000;
	_ParRef.current		= 150.0;
	_ParRef.stop_mux	= 0;
	_ParRef.dis_mux_in	= 0;
	_ParRef.stop_in		= ESTOP_UNUSED;
	_ParRef.stop_level	= 0;
	_ParRef.estop_in    = ESTOP_UNUSED;
	_ParRef.estop_level = 0;
	_ParRef.checkEncoder=TRUE;
	
	_ParZ_down.speed		= 10000;
	_ParZ_down.accel		= 5000;
	_ParZ_down.current		= 200.0; // 150.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.stop_in		= ESTOP_UNUSED;
	_ParZ_down.stop_level	= 0;
	_ParZ_down.estop_in     = ESTOP_UNUSED;
	_ParZ_down.estop_level  = 0;
	_ParZ_down.checkEncoder = TRUE;

	_ParZ_cap.speed			= 2000;
	_ParZ_cap.accel			= 2000;
	_ParZ_cap.current		= 100.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= 0;
	_ParZ_cap.stop_in       = ESTOP_UNUSED;
	_ParZ_cap.stop_level    = 0;
	_ParZ_cap.estop_in      = ESTOP_UNUSED;
	_ParZ_cap.estop_level   = 0;
	_ParZ_cap.checkEncoder  = TRUE;
	
	_tx801_set_ventilators(0);
}

//--- tx801_main ------------------------------------------------------------------
void tx801_main(int ticks, int menu)
{
	int motor, ok;
	motor_main(ticks, menu);
	
	RX_TestTableStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_TestTableStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
	RX_TestTableStatus.info.headUpInput_2 = fpga_input(HEAD_UP_IN_2);
	RX_TestTableStatus.info.headUpInput_3 = fpga_input(HEAD_UP_IN_3);
	RX_TestTableStatus.posZ				  = TX_REF_HEIGHT - _steps_2_micron(motor_get_step(MOTOR_Z_0));

	{
		int i;
		RX_TestTableStatus.inputs = Fpga.stat->input;
		for (i=0; i<MAX_STEPPER_MOTORS; i++)
		{
			RX_TestTableStatus.motor[i].motor_pos	= TX_REF_HEIGHT-_steps_2_micron(Fpga.stat->statMot[i].position);
			RX_TestTableStatus.motor[i].encoder_pos = TX_REF_HEIGHT-_incs_2_micron(Fpga.encoder[i].pos);
			if (Fpga.stat->moving & (0x01<<i))
			{
				if(Fpga.par->mot_bwd & (0x01<<i)) RX_TestTableStatus.motor[i].state = MOTOR_STATE_MOVING_FWD;
				else							  RX_TestTableStatus.motor[i].state = MOTOR_STATE_MOVING_BWD;
			}
			else if (Fpga.stat->statMot[i].err_estop & ENC_ESTOP_ENC) RX_TestTableStatus.motor[i].state = MOTOR_STATE_BLOCKED;
			else RX_TestTableStatus.motor[i].state = MOTOR_STATE_IDLE;
		}		
	}

	if (RX_TestTableStatus.info.moving)
	{
		RX_TestTableStatus.info.z_in_ref   = FALSE;
		RX_TestTableStatus.info.z_in_print = FALSE;
		RX_TestTableStatus.info.z_in_cap   = FALSE;			
	}
	if (_CmdRunning && motors_move_done(MOTOR_Z_BITS)) 
	{
		RX_TestTableStatus.info.moving = FALSE;
		if (_CmdRunning == CMD_CAP_REFERENCE) 
		{
			if (motor_error(MOTOR_Z_BITS))
			{	
				Error(ERR_CONT, 0, "LIFT: %s: motor %s blocked", ctrl_cmd_name(_CmdRunning), _motor_name(motor));
				RX_TestTableStatus.info.ref_done = FALSE;
			}
			else
			{
				motors_reset(MOTOR_Z_BITS);				
				for (motor=0, ok=TRUE; motor<MOTOR_Z_CNT; motor++)
				{
					if (!fpga_input(motor))	
					{
						Error(ERR_CONT, 0, "Referencing motor %s: Sensor not covered", _motor_name(motor));
						ok = FALSE;	
					}
				}
					
				RX_TestTableStatus.info.ref_done = ok;
			}
		}
		else
		{
			for (motor=0, ok=TRUE; motor<MOTOR_Z_CNT; motor++)
			{
				if (motor_error(motor))
				{
					Error(ERR_CONT, 0, "tx801: %s: motor %s blocked", ctrl_cmd_name(_CmdRunning), _motor_name(motor));
					ok=FALSE;
				}				
			}
			if (!ok) RX_TestTableStatus.info.ref_done = FALSE;							
		}
		RX_TestTableStatus.info.z_in_ref    = ((_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS) && RX_TestTableStatus.info.ref_done);
		RX_TestTableStatus.info.z_in_print  = (_CmdRunning==CMD_CAP_PRINT_POS && RX_TestTableStatus.info.ref_done);
		RX_TestTableStatus.info.z_in_cap    = (_CmdRunning==CMD_CAP_CAPPING_POS);
		if (_CmdRunning==CMD_CAP_REFERENCE && _PrintPos_New) 
		{
			_tx801_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
			_PrintPos_Act=_PrintPos_New;
			_PrintPos_New=0;
		}
		else {
			RX_TestTableStatus.info.move_tgl = !RX_TestTableStatus.info.move_tgl;
			_CmdRunning = FALSE;
		}
	}	
}


//--- _motor_name ------------------------------
static char *_motor_name(int no)
{
	switch(no)
	{
	case 0:	return "Front Left";
	case 1: return "Back Left";
	case 2: return "Back Right";
	case 3: return "Front Right";
	default: return "";
	}
}

//--- _tx801_display_status ---------------------------------------------------------
static void _tx801_display_status(void)
{
	term_printf("CAPPING Unit ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n",	RX_TestTableStatus.info.moving, _CmdRunning);	
	term_printf("Head UP Sensor: %d%d%d%d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1), fpga_input(HEAD_UP_IN_2), fpga_input(HEAD_UP_IN_3));	
	term_printf("reference done: %d\n",	RX_TestTableStatus.info.ref_done);
	term_printf("z in reference: %d\n",	RX_TestTableStatus.info.z_in_ref);
	term_printf("z in print:     %d\n",	RX_TestTableStatus.info.z_in_print);
	term_printf("z in capping:   %d\n",	RX_TestTableStatus.info.z_in_cap);
	term_printf("z position in micro:   %d\n", RX_TestTableStatus.posZ);	
	term_printf("move toggle:    %d\n", RX_TestTableStatus.info.move_tgl);
	term_printf("\n");
}

//--- tx801_menu --------------------------------------------------
int tx801_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int pos=10000;

	_tx801_display_status();
	
	term_printf("MENU tx801 -------------------------\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("R: Reference\n");
	term_printf("c: move to cap\n");
	term_printf("u: move to UP position\n");
	term_printf("p: move to print\n");
	term_printf("z: move by <steps>\n");
	term_printf("v: set ventilator (%)\n");	
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,			NULL);	break;
		case 'R': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,		NULL);	break;
		case 'r': motor_reset(atoi(&str[1]));											break;
		case 'c': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS,	NULL);	break;
		case 'p': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,		&pos);	break;
		case 'u': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS,			NULL);	break;
		case 'z': _tx801_motor_z_test(atoi(&str[1]));									break;
		case 'm': _tx801_motor_test(str[1]-'0', atoi(&str[2]));							break;			
		case 'v': _tx801_set_ventilators(atoi(&str[1]));								break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}


//--- _tx801_do_reference ----------------------------------------------------------------
static void _tx801_do_reference(void)
{
	motors_stop	(MOTOR_Z_BITS);
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, 0.0, 0.0);

	_CmdRunning  = CMD_CAP_REFERENCE;
	RX_TestTableStatus.info.moving = TRUE;
	motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, -500000, TRUE);
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5+STEPS_REV/DIST_REV*micron);
}

//---_steps_2_micron --------------------------------------------------------------
static int  _steps_2_micron(int steps)
{
	return (int)(DIST_REV * steps / STEPS_REV + 0.5);
}

//--- _incs_2_micron ----------------------------
static int  _incs_2_micron(int incs)
{
	return (int)(DIST_REV * incs / INCS_REV + 0.5);
}

//--- _tx801_move_to_pos ---------------------------------------------------------------
static void _tx801_move_to_pos(int cmd, int pos)
{
	_CmdRunning  = cmd;
	RX_TestTableStatus.info.moving = TRUE;
	motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
}

//--- _tx801_set_ventilators ---------------------------------------------------------
static void _tx801_set_ventilators(int value)
{
	Error(LOG, 0, "Set Ventilators to %d%%", value);
	
	if(value<100) value=(0x10000 * value) / 100;
	else value=0x10000-1;
	int i;
	for (i=0; i<6; i++) Fpga.par->pwm_output[i] = value;
}

//--- tx801_handle_ctrl_msg -----------------------------------
int  tx801_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	INT32 pos, steps, voltage;
		
	switch(msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
									break;
		
	case CMD_CAP_STOP:				motors_stop(MOTOR_Z_BITS);
									_CmdRunning = 0;
									break;	

	case CMD_CAP_REFERENCE:			_PrintPos_New=0;
									motors_reset(MOTOR_Z_BITS);
									_tx801_do_reference();	
									break;

	case CMD_CAP_PRINT_POS:			pos   = (*((INT32*)pdata));
									if (pos<TX_PRINT_POS_MIN) pos=TX_PRINT_POS_MIN;
									if (!_CmdRunning && (!RX_TestTableStatus.info.z_in_print || steps!=_PrintPos_Act))
									{
										_PrintPos_New = _micron_2_steps(TX_REF_HEIGHT - pos);
										if (RX_TestTableStatus.info.ref_done) _tx801_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
										else								  _tx801_do_reference();
									}
									break;
		
	case CMD_CAP_UP_POS:			if (!_CmdRunning)
									{
										if (RX_TestTableStatus.info.ref_done) _tx801_move_to_pos(CMD_CAP_UP_POS, POS_UP);
										else								  _tx801_do_reference();
									}
									break;

	case CMD_CAP_CAPPING_POS:		if (!_CmdRunning)
									{
										_CmdRunning  = msgId;
										RX_TestTableStatus.info.moving = TRUE;
										steps = _micron_2_steps(TX_REF_HEIGHT - RX_StepperCfg.cap_height);
										motors_move_to_step	(MOTOR_Z_BITS,  &_ParZ_cap, steps);
									}
									break;
	
	case CMD_STEPPER_TEST:			if (!_CmdRunning)
									{
										SStepperMotorTest *ptest = (SStepperMotorTest*)pdata;
										_CmdRunning  = msgId;
										RX_TestTableStatus.info.z_in_print  = FALSE;
										_tx801_motor_test(ptest->motorNo, -1*_micron_2_steps(ptest->microns));
									}
									break;
		
	case CMD_CAP_VENT:				voltage = (*((INT32*)pdata));
									_tx801_set_ventilators(voltage);				
									break;
		
	default:						break;
	}
	return REPLY_OK;
}

//--- _tx801_motor_z_test ----------------------------------------------------------------------
void _tx801_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_TestTableStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, steps, TRUE);
}

//--- _tx801_motor_test ---------------------------------
static void _tx801_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;
	int i;

	par.speed		= 1000;
	par.accel		= 1000;
	par.current		= 250.0;
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	
	_CmdRunning = 1; // TEST
	RX_TestTableStatus.info.moving = TRUE;
	
	motors_config(motors, CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motors, &par, steps, FALSE);			
}

