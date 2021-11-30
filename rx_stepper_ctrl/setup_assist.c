// ****************************************************************************
//
//	DIGITAL PRINTING - sa.c
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
#include "setup_assist.h"

#define MOTOR_SCAN			0
#define MOTOR_LIFT			1

#define MOTOR_ALL_BITS		0x03

#define CURRENT_HOLD_SCAN	50
#define CURRENT_HOLD_LIFT	20

#define LIFT_DIST			10000		// um

#define STEPS_REV_SCAN		(200*STEPS)	// steps per motor revolution * 16 times oversampling
#define DIST_REV_SCAN		99000.0		// moving distance per revolution [um]

#define STEPS_REV_LIFT		(200*STEPS)	// steps per motor revolution * 16 times oversampling
#define DIST_REV_LIFT		1000		// moving distance per revolution [um]

#define SCAN_SPEED			5000

//Inputs
#define SCAN_REF			0

static SMovePar	_ParScan;
static SMovePar _ParScanRef;
static SMovePar	_ParLift;
static int		_Step;
static char		_CmdName[32];
static int		_ScanPos_New=0;
static int		_ScanSpeed=0;
static int		_ScanPos_Act=0;
static int		_CmdRunning_Z = 0;


//--- prototypes --------------------------------------------
static void _sa_motor_z_test(int steps);
static void _sa_motor_test  (int motor, int steps);
static void _sa_do_reference(void);
static void _sa_move_to_pos(int cmd, int pos, int speed);
static int  _micron_2_steps_scan(int micron);
static int  _steps_2_microns_scan(int steps);
static int	_micron_2_steps_lift(int micron);

//--- sa_init --------------------------------------
void sa_init(void)
{
    motor_config(MOTOR_SCAN, CURRENT_HOLD_SCAN, STEPS_REV_SCAN, DIST_REV_SCAN, 1);
    motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);

    memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
    _ParScan.speed			= SCAN_SPEED;
    _ParScan.accel			= 32000;
    _ParScan.current_acc	= 420.0;
    _ParScan.current_run	= 300.0;
    _ParScan.stop_mux		= 0;
    _ParScan.dis_mux_in		= 0;
    _ParScan.encCheck		= chk_off;

    _ParScanRef.speed = 1000;
    _ParScanRef.accel = 32000;
    _ParScanRef.current_acc = 420.0;
    _ParScanRef.current_run = 300.0;
    _ParScanRef.stop_mux = 0;
    _ParScanRef.estop_in_bit[MOTOR_SCAN] = 1 << SCAN_REF;
    _ParScanRef.estop_level = FALSE;
    _ParScanRef.dis_mux_in = 0;
    _ParScanRef.encCheck = chk_off;

    _ParLift.speed = 10000;
    _ParLift.accel = 32000;
    _ParLift.current_acc = 60.0;		// max 60 (60 means 0.6A)
    _ParLift.current_run = 60.0;        // max 60 (60 means 0.6A)
    _ParLift.stop_mux = 0;
    _ParLift.dis_mux_in = 0;
    _ParLift.encCheck		= chk_off;
}

//--- sa_main ------------------------------------------------------------------
void sa_main(int ticks, int menu)
{
	int motor;
	motor_main(ticks, menu);
	
    RX_StepperStatus.info.headUpInput_0 = !fpga_input(SCAN_REF);

    if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.x_in_ref   = FALSE;
		RX_StepperStatus.posX = _steps_2_microns_scan(motor_get_step(MOTOR_SCAN));
    }
	else RX_StepperStatus.posX = _ScanPos_Act;

	if (_CmdRunning_Z && motor_move_done(MOTOR_LIFT))
	{
		if (motor_error(MOTOR_LIFT))
		{
			Error(ERR_CONT, 0, "LIFT: Command %d: Motor LIFT blocked", _CmdRunning_Z);			
			RX_StepperStatus.info.ref_done = FALSE;							
		}

		switch(_CmdRunning_Z)
		{
        case CMD_SA_UP:		TrPrintfL(TRUE, "CMD_SA_UP DONE");
							RX_StepperStatus.info.z_in_up=TRUE;  
							RX_StepperStatus.info.z_in_down=FALSE;
							break;

        case CMD_SA_DOWN:	TrPrintfL(TRUE, "CMD_SA_DOWN DONE");
							RX_StepperStatus.info.z_in_up=FALSE; 
							RX_StepperStatus.info.z_in_down=TRUE; 
							break;
		}
		_CmdRunning_Z = 0;
	}

//	if (RX_StepperStatus.cmdRunning==0) RX_StepperStatus.info.moving = FALSE;
    if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{
		RX_StepperStatus.info.x_in_ref      = RX_StepperStatus.info.headUpInput_0;

		RX_StepperStatus.info.moving	    = FALSE;
		if (RX_StepperStatus.cmdRunning == CMD_SA_REFERENCE) 
		{
			_sa_do_reference();
		}
        else 
		{
			if (motor_error(MOTOR_SCAN))
			{
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor SCAN blocked", _CmdName);			
				RX_StepperStatus.info.ref_done = FALSE;							
			}
			else
			{
				if (RX_StepperStatus.cmdRunning == CMD_SA_MOVE)
				{
					RX_StepperStatus.posY[0]++;
					TrPrintfL(TRUE, "MOVE DONE [%d]: pos=%d", RX_StepperStatus.posY[0], RX_StepperStatus.posX);
					if (_ScanPos_New>=0) _ScanPos_Act = _ScanPos_New;
					else				 _ScanPos_Act = RX_StepperStatus.posX;
					RX_StepperStatus.posX = _ScanPos_Act;
					_ScanPos_New = 0;

				}
				else if (_ScanPos_New) _sa_move_to_pos(CMD_SA_MOVE, _ScanPos_New, _ScanSpeed);
			}
			RX_StepperStatus.cmdRunning = FALSE;
			ctrl_send_status();
		}
	}	
}

//--- _sa_display_status ---------------------------------------------------------
static void _sa_display_status(void)
{
	term_printf("SETUP ASSIST ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n",	RX_StepperStatus.info.moving, RX_StepperStatus.cmdRunning);
	term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
	term_printf("z in reference: %d\n",	RX_StepperStatus.info.x_in_ref);
	term_printf("z in up:        %d\n",	RX_StepperStatus.info.z_in_up);
	term_printf("z in down:      %d\n",	RX_StepperStatus.info.z_in_down);
	term_printf("posX:           %d\n",	RX_StepperStatus.posX);	
	term_printf("\n");
}

//--- sa_menu --------------------------------------------------
int sa_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	SetupAssist_MoveCmd msg;

	_sa_display_status();
	
	term_printf("LB SA MENU -------------------------\n");
	term_printf("s: STOP\n");
    term_printf("R: Reference\n");
	term_printf("r<n>: reset motor<n>\n");	
    term_printf("p<n>: move scan to pos <n> um\n");
	term_printf("u: move to UP position\n");
	term_printf("d: move to DOWN position\n");
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
		case 's': msg.hdr.msgId=CMD_SA_STOP;		sa_handle_ctrl_msg(INVALID_SOCKET, &msg); break;
		case 'R': msg.hdr.msgId=CMD_SA_REFERENCE;	sa_handle_ctrl_msg(INVALID_SOCKET, &msg); break;
		case 'r': motor_reset(atoi(&str[1]));									   break;
		case 'p': 
					msg.hdr.msgId = CMD_SA_MOVE;
					msg.pos		  = atoi(&str[1]);
					msg.speed	  = 0;
					sa_handle_ctrl_msg(INVALID_SOCKET, &msg);
					break;
		case 'u': msg.hdr.msgId=CMD_SA_UP;	 sa_handle_ctrl_msg(INVALID_SOCKET, &msg); break;
		case 'd': msg.hdr.msgId=CMD_SA_DOWN; sa_handle_ctrl_msg(INVALID_SOCKET, &msg); break;
		case 'z': _sa_motor_z_test(atoi(&str[1])); break;
        case 'o': Fpga.par->output ^= (1 << atoi(&str[1]));	break;
		case 'm': _sa_motor_test(str[1]-'0', atoi(&str[2])); break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}


//--- _sa_do_reference ----------------------------------------------------------------
static void _sa_do_reference(void)
{
	TrPrintfL(TRUE, "_sa_do_reference(%d)", _Step);
	switch(_Step)
	{
    case 0:	motors_stop(1<<MOTOR_SCAN);
			motors_reset(1<<MOTOR_SCAN);
			motors_init_reset();
			motor_config(MOTOR_SCAN, CURRENT_HOLD_SCAN, STEPS_REV_SCAN, DIST_REV_SCAN, 1);
			motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);
			RX_StepperStatus.info.z_in_up   = FALSE;
			RX_StepperStatus.info.z_in_down = FALSE;
			RX_StepperStatus.cmdRunning  = CMD_SA_REFERENCE;
			_Step=1;

	case 1: _Step=2;
			if (!RX_StepperStatus.info.z_in_up)
			{
				_CmdRunning_Z = CMD_SA_UP;
				motors_move_to_step(1 << MOTOR_LIFT, &_ParLift, LIFT_DIST);
				break;				
			}

    case 2: _Step=3;
			RX_StepperStatus.info.moving = TRUE;
			motors_move_by_step(1 << MOTOR_SCAN, &_ParScanRef, -500000, TRUE);
			break;

    case 3: RX_StepperStatus.cmdRunning=0;
			RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0;
			if (!RX_StepperStatus.info.headUpInput_0) Error(ERR_CONT, 0, "SA: Command REFERENCE: End Sensor NOT HIGH");
			motors_reset(MOTOR_ALL_BITS);
			_ScanPos_Act=0;
			if (_ScanPos_New) _sa_move_to_pos(CMD_SA_MOVE, _ScanPos_New, _ScanSpeed);
	}
}

//---_micron_2_steps_seldge --------------------------------------------------------------
static int  _micron_2_steps_scan(int micron)
{
	int steps=(int)(0.5 + (double)micron*STEPS_REV_SCAN /  DIST_REV_SCAN);
	return steps;
}

static int  _steps_2_microns_scan(int steps)
{
	int microns=(int)(0.5 + (double)steps * DIST_REV_SCAN / STEPS_REV_SCAN);
	return microns;
}

//---_micron_2_sptes_lift --------------------------------------------------------------
static int _micron_2_steps_lift(int micron)
{
    return (int)(0.5 + STEPS_REV_LIFT / DIST_REV_LIFT * micron);
}

//--- _sa_move_to_pos ---------------------------------------------------------------
static void _sa_move_to_pos(int cmd, int pos, int speed)
{
    RX_StepperStatus.cmdRunning  = cmd;
	RX_StepperStatus.info.moving = TRUE;
	if (speed)	_ParScan.speed = speed;
	else		_ParScan.speed = SCAN_SPEED;
    motors_move_to_step(1 << MOTOR_SCAN, &_ParScan, _micron_2_steps_scan(pos));
}

//--- sa_handle_ctrl_msg -----------------------------------
int  sa_handle_ctrl_msg(RX_SOCKET socket, void *pmsg)
{	
	SetupAssist_MoveCmd *pcmd = (SetupAssist_MoveCmd*)pmsg;

	switch(pcmd->hdr.msgId)
	{
    case CMD_SA_STOP:				strcpy(_CmdName, "CMD_SA_STOP");
									TrPrintfL(TRUE, "CMD_SA_STOP pos=%d", _steps_2_microns_scan(motor_get_step(MOTOR_SCAN)));
                                    _ScanPos_New = -1;
									motors_stop(1<<MOTOR_SCAN);
									break;

    case CMD_SA_REFERENCE:			strcpy(_CmdName, "CMD_SA_REFERENCE");
									_ScanPos_New=0;
									_ScanSpeed=0;
                                    RX_StepperStatus.cmdRunning = pcmd->hdr.msgId;
									_Step=0;
									_sa_do_reference();	
									break;

    case CMD_SA_MOVE:				strcpy(_CmdName, "CMD_SA_MOVE_SCAN");
									TrPrintfL(TRUE, "CMD_SA_MOVE");
									_ScanPos_New = pcmd->pos;
									_ScanSpeed   = pcmd->speed;
									TrPrintfL(TRUE, "CMD_SA_MOVE [%d]: from %d to %d, speed=%d", RX_StepperStatus.posY[0]+1, RX_StepperStatus.posX, _ScanPos_New, _ScanSpeed);
                                    if (RX_StepperStatus.info.ref_done) _sa_move_to_pos(CMD_SA_MOVE, _ScanPos_New, _ScanSpeed);
									else								_sa_do_reference();
									break;

    case CMD_SA_UP:					strcpy(_CmdName, "CMD_SA_UP");
									TrPrintfL(TRUE, "CMD_SA_UP");
									if (!RX_StepperStatus.info.z_in_up)
									{
                                        _CmdRunning_Z = pcmd->hdr.msgId;
                                        motors_move_by_step(1<<MOTOR_LIFT, &_ParLift, LIFT_DIST, FALSE);
                                    }
									break;

    case CMD_SA_DOWN:				strcpy(_CmdName, "CMD_SA_DOWN");
									TrPrintfL(TRUE, "CMD_SA_DOWN");
									if (!RX_StepperStatus.info.z_in_down)
									{
                                        _CmdRunning_Z = pcmd->hdr.msgId;
										motors_move_by_step(1<<MOTOR_LIFT, &_ParLift, -LIFT_DIST, FALSE);
                                    }
									break;

    case CMD_ERROR_RESET:			fpga_stepper_error_reset();
									break;
                                    
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", pcmd->hdr.msgId); break;
	}
}

//--- _sa_motor_z_test ----------------------------------------------------------------------
void _sa_motor_z_test(int steps)
{	
	RX_StepperStatus.cmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(1<<MOTOR_LIFT, &_ParLift, steps, TRUE);
}

//--- _sa_motor_test ---------------------------------
static void _sa_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;
	int i;

	memset(&par, 0, sizeof(par));
	par.speed		= 10000;
	par.accel		= 32000;
    if (motorNo == MOTOR_SCAN)
    {
        par.current_acc = 420.0;
        par.current_run = 420.0;
    }
    else
    {
        par.current_acc = 60.0;
        par.current_run = 60.0;
    }
    par.stop_mux = 0;
    par.estop_in_bit[MOTOR_SCAN] = (1 << SCAN_REF);
    par.estop_level = FALSE;
	par.dis_mux_in	= 0;
	par.encCheck	= chk_off;
	
	RX_StepperStatus.cmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;

    if (motorNo == MOTOR_SCAN)
        motor_config(MOTOR_SCAN, CURRENT_HOLD_SCAN, STEPS_REV_SCAN, DIST_REV_SCAN, 1);
    else if (motorNo == MOTOR_LIFT)
        motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);
    
    motors_move_by_step(motors, &par, steps, FALSE);
}