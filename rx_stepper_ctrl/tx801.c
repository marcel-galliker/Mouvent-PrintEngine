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
static int		_CapHight = 0;
static int		_WetWipeHight = 0;
static int		_WipeHight = 0;
static int		_VacuumHight = 0;
static int		_VentCtrlDelay=0;
static int		_VentSpeed = 0;
static int		_VentValue[6];
static int		_LastVentValue[6];
static int		_VentErrorCnt[6];

//--- prototypes --------------------------------------------
static void _tx801_control_vents(void);
static void _tx801_motor_z_test(int steps);
static void _tx801_motor_test  (int motor, int steps);
static void _tx801_do_reference(void);
static void _tx801_do_ctrlMode(EnFluidCtrlMode ctrlMode);
static void _tx801_move_to_pos(int cmd, int pos);
static void _tx801_set_ventilators(int value);
static int  _micron_2_steps(int micron);// microns to motor steps
static int  _steps_2_micron(int steps); // motor steps to microns
static int  _incs_2_micron(int steps);  // encoder_incs to microns
static char	*_motor_name(int no);
static int _tx801_motor_down(RX_SOCKET socket, int msgId, int length);

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
	{
		int i;
		for (i=0; i<SIZEOF(_LastVentValue); i++)
			 _LastVentValue[i]=50;
	}
}

//--- tx801_main ------------------------------------------------------------------
void tx801_main(int ticks, int menu)
{
	int motor, ok;
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
	RX_StepperStatus.info.headUpInput_2 = fpga_input(HEAD_UP_IN_2);
	RX_StepperStatus.info.headUpInput_3 = fpga_input(HEAD_UP_IN_3);
	RX_StepperStatus.posZ				  = TX_REF_HEIGHT - _steps_2_micron(motor_get_step(MOTOR_Z_0));

	{
		int i;
		RX_StepperStatus.inputs = Fpga.stat->input;
		for (i=0; i<MAX_STEPPER_MOTORS; i++)
		{
			RX_StepperStatus.motor[i].motor_pos	= TX_REF_HEIGHT-_steps_2_micron(Fpga.stat->statMot[i].position);
			RX_StepperStatus.motor[i].encoder_pos = TX_REF_HEIGHT-_incs_2_micron(Fpga.encoder[i].pos);
			if (Fpga.stat->moving & (0x01<<i))
			{
				if(Fpga.par->mot_bwd & (0x01<<i)) RX_StepperStatus.motor[i].state = MOTOR_STATE_MOVING_FWD;
				else							  RX_StepperStatus.motor[i].state = MOTOR_STATE_MOVING_BWD;
			}
			else if (Fpga.stat->statMot[i].err_estop & ENC_ESTOP_ENC) RX_StepperStatus.motor[i].state = MOTOR_STATE_BLOCKED;
			else RX_StepperStatus.motor[i].state = MOTOR_STATE_IDLE;
		}		
	}

	if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;			
	}
	if (_CmdRunning && motors_move_done(MOTOR_Z_BITS)) 
	{
		RX_StepperStatus.info.moving = FALSE;
		if (_CmdRunning == CMD_CAP_REFERENCE) 
		{
			if (motor_error(MOTOR_Z_BITS))
			{	
				Error(ERR_CONT, 0, "LIFT: %s: motor %s blocked", ctrl_cmd_name(_CmdRunning), _motor_name(motor));
				RX_StepperStatus.info.ref_done = FALSE;
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
					
				RX_StepperStatus.info.ref_done = ok;
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
			if (!ok) RX_StepperStatus.info.ref_done = FALSE;							
		}
		RX_StepperStatus.info.z_in_ref    = ((_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS) && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_print  = (_CmdRunning==CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_cap    = (_CmdRunning==CMD_CAP_CAPPING_POS);
		if (_CmdRunning==CMD_CAP_REFERENCE && _PrintPos_New) 
		{
			_tx801_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
			_PrintPos_Act=_PrintPos_New;
			_PrintPos_New=0;
		}
		else {
			RX_StepperStatus.info.move_tgl = !RX_StepperStatus.info.move_tgl;
			_CmdRunning = FALSE;
		}
	}
//	_tx801_control_vents();	
}

//--- _tx801_control_vents -------------------
static void _tx801_control_vents(void)
{
	static int _lastTime=0;
	int i;
	int time = rx_get_ticks();
	
	if (time-_lastTime>100)
	{
		if(_VentSpeed)
		{			
			if (time>_VentCtrlDelay)
			{
				for (i=0; i<6; i++)
				{
					if      (fpga_pwm_speed(i) - _VentSpeed > 2)  _VentValue[i] -= 5;
					else if (fpga_pwm_speed(i) - _VentSpeed < -2) _VentValue[i] += 5;
					if(_VentValue[i] >= 100 && ++_VentErrorCnt[i] == 1000) 
						Error(ERR_CONT, 0, "Drop Suction Ventilator %d blocked", i + 1);
					else if (_VentErrorCnt[i] && _VentErrorCnt[i]<1000) _VentErrorCnt[i]--;
				}					
			}
		}
		else
		{
			memset(_VentValue, 0, sizeof(_VentValue));
			memset(_VentErrorCnt, 0, sizeof(_VentErrorCnt));
		}
		for (i=0; i<6; i++) 
		{
			if (_VentValue[i]<30)      Fpga.par->pwm_output[i] = 0;
			else if(_VentValue[i]<100) Fpga.par->pwm_output[i] = (0x10000 * _VentValue[i]) / 100;
			else Fpga.par->pwm_output[i]=0x10000-1;
		}							
		_lastTime = time;
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
	term_printf("moving:         %d		cmd: %08x\n",	RX_StepperStatus.info.moving, _CmdRunning);	
	term_printf("Head UP Sensor: %d%d%d%d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1), fpga_input(HEAD_UP_IN_2), fpga_input(HEAD_UP_IN_3));	
	term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
	term_printf("z in reference: %d\n",	RX_StepperStatus.info.z_in_ref);
	term_printf("z in print:     %d\n",	RX_StepperStatus.info.z_in_print);
	term_printf("z in capping:   %d\n",	RX_StepperStatus.info.z_in_cap);
	term_printf("z position in micro:   %d\n", RX_StepperStatus.posZ);	
	term_printf("move toggle:    %d\n", RX_StepperStatus.info.move_tgl);
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
	//term_printf("c: move to cap\n");
	term_printf("u: move to UP position\n");
	term_printf("p: move to print\n");
	term_printf("z: move by <steps>\n");
	term_printf("v: set ventilator (%)\n");	
	term_printf("t<n>: test move down n in tenth of millimeter in absolute position from ref position\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("c: move to capping hight  %03d.%d         C<n>: set capping hight in absolute position from ref position\n",  _CapHight/10, _CapHight%10);
	term_printf("e: move to wet wipe hight %03d.%d         E<n>: set wet wipe hight in absolute position from ref position\n", _CapHight/10, _CapHight%10);
	term_printf("w: move to wipe hight     %03d.%d         W<n>: set wipe hight in absolute position from ref position\n",	   _WipeHight/10, _WipeHight%10);
	term_printf("a: move to vacuum hight   %03d.%d         A<n>: set vacuum hight in absolute position from ref position\n",   _VacuumHight/10, _VacuumHight%10);
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,			NULL);		break;
		case 'R': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,		NULL);		break;
		case 'r': motor_reset(atoi(&str[1]));												break;
		//case 'c': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS,	NULL);		break;
		case 'p': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,		&pos);		break;
		case 'u': tx801_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS,			NULL);		break;
		case 'z': _tx801_motor_z_test(atoi(&str[1]));										break;
		case 'm': _tx801_motor_test(str[1]-'0', atoi(&str[2]));								break;			
		case 'v': _tx801_set_ventilators(atoi(&str[1]));									break;
		case 't': _tx801_motor_down(INVALID_SOCKET, CMD_CAP_CAPPING_POS, atoi(&str[1]));	break;
		case 'c': _tx801_motor_down(INVALID_SOCKET, CMD_CAP_CAPPING_POS, _CapHight);		break;
		case 'e': _tx801_motor_down(INVALID_SOCKET, CMD_CAP_CAPPING_POS, _WetWipeHight);	break;
		case 'w': _tx801_motor_down(INVALID_SOCKET, CMD_CAP_CAPPING_POS, _WipeHight);		break;
		case 'a': _tx801_motor_down(INVALID_SOCKET, CMD_CAP_CAPPING_POS, _VacuumHight);		break;
		case 'C': _CapHight = atoi(&str[1]);												break;
		case 'E': _WetWipeHight = atoi(&str[1]);											break;
		case 'W': _WipeHight = atoi(&str[1]);												break;
		case 'A': _VacuumHight = atoi(&str[1]);												break;
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
	RX_StepperStatus.info.moving = TRUE;
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
	RX_StepperStatus.info.moving = TRUE;
	motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
}

//--- _tx801_set_ventilators ---------------------------------------------------------
static void _tx801_set_ventilators(int value)
{
//	Error(LOG, 0, "Set Ventilators to %d%%", value);
	
	/*
	if(value && !_VentSpeed) memcpy(_VentValue, _LastVentValue, sizeof(_VentValue));
	else					 memcpy(_LastVentValue, _VentValue, sizeof(_LastVentValue));
	_VentSpeed = value;
	_VentCtrlDelay = rx_get_ticks()+1000;
	_tx801_control_vents();
	*/

	if(value<100) value=(0x10000 * value) / 100;
	else value=0x10000-1;
	int i;
	for (i=0; i<6; i++) Fpga.par->pwm_output[i] = value;
}

//--- _tx801_do_ctrlMode -----------------------------------------
static void _tx801_do_ctrlMode(EnFluidCtrlMode ctrlMode)
{
//	sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
}

//--- tx801_handle_ctrl_msg -----------------------------------
int  tx801_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	INT32 pos, steps, voltage;
	SValue	*pvalue;
		
	switch(msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	
									break;
		
	case CMD_CAP_STOP:				motors_stop(MOTOR_Z_BITS);
									_CmdRunning = 0;
									break;	

	case CMD_CAP_REFERENCE:			_PrintPos_New=0;
									motors_reset(MOTOR_Z_BITS);
									_tx801_do_reference();	
									break;

	case CMD_CAP_PRINT_POS:			pos   = (*((INT32*)pdata));
									if (pos<TX_PRINT_POS_MIN) 
									{
										pos=TX_PRINT_POS_MIN;
										Error(WARN, 0, "CMD_CAP_PRINT_POS set to MIN pos=%d.%03d", pos/1000, pos%1000);
									}
									if (!_CmdRunning && (!RX_StepperStatus.info.z_in_print || steps!=_PrintPos_Act))
									{
										_PrintPos_New = _micron_2_steps(TX_REF_HEIGHT - pos);
										if (RX_StepperStatus.info.ref_done) _tx801_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
										else								_tx801_do_reference();
									}
									break;
		
	case CMD_CAP_UP_POS:			if (!_CmdRunning)
									{
										if (RX_StepperStatus.info.ref_done) _tx801_move_to_pos(CMD_CAP_UP_POS, POS_UP);
										else								_tx801_do_reference();
									}
									break;

	case CMD_CAP_CAPPING_POS:		if (!_CmdRunning)
									{
										_CmdRunning  = msgId;
										RX_StepperStatus.info.moving = TRUE;
										steps = _micron_2_steps(TX_REF_HEIGHT - RX_StepperCfg.cap_height);
										motors_move_to_step	(MOTOR_Z_BITS,  &_ParZ_cap, steps);
									}
									break;
	
	case CMD_STEPPER_TEST:			if (!_CmdRunning)
									{
										SStepperMotorTest *ptest = (SStepperMotorTest*)pdata;
										_CmdRunning  = msgId;
										RX_StepperStatus.info.z_in_print  = FALSE;
										_tx801_motor_test(ptest->motorNo, -1*_micron_2_steps(ptest->microns));
									}
									break;
		
	case CMD_CAP_VENT:				voltage = (*((INT32*)pdata));
									_tx801_set_ventilators(voltage);				
									break;
		
	case CMD_FLUID_CTRL_MODE:		pvalue = (SValue*) pdata;
									_tx801_do_ctrlMode((EnFluidCtrlMode)pvalue->value);
									break;
		
	default:						break;
	}
	return REPLY_OK;
}

//--- _tx801_motor_z_test ----------------------------------------------------------------------
void _tx801_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
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
	RX_StepperStatus.info.moving = TRUE;
	
	motors_config(motors, CURRENT_HOLD, 0.0, 0.0);
	motors_move_by_step(motors, &par, steps, FALSE);			
}

//--- _tx801_motor_down ---------------------------------------
static int _tx801_motor_down(RX_SOCKET socket, int msgId, int length)
{
	INT32 steps;
	if (!_CmdRunning)
	{
		_CmdRunning  = msgId;
		RX_StepperStatus.info.moving = TRUE;
  		steps = _micron_2_steps(100 * length);
		motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, steps);
	}
	return REPLY_OK;
}


