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
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "sa.h"

#define MOTOR_SLEDGE		0
#define MOTOR_LIFT			1

#define MOTOR_ALL_BITS		0x03

#define CURRENT_HOLD_SLEDGE	50
#define CURRENT_HOLD_LIFT	20

#define POS_UP				5000		// um

#define STEPS_REV_SLEDGE	200*STEPS	// steps per motor revolution * 16 times oversampling
#define DIST_REV_SLEDGE		99000.0		// moving distance per revolution [um]

#define STEPS_REV_LIFT		200*STEPS	// steps per motor revolution * 16 times oversampling
#define DIST_REV_LIFT		1000		// moving distance per revolution [um]

//Inputs
#define SLEDGE_REF			0

static SMovePar	_ParSledge;
static SMovePar _ParSledgeRef;
static SMovePar	_ParLift;
static char		_CmdName[32];
static int		_SledgePos_New=0;
static int		_SledgePos_Act=0;


//--- prototypes --------------------------------------------
static void _sa_motor_z_test(int steps);
static void _sa_motor_test  (int motor, int steps);
static void _sa_do_reference(void);
static void _sa_move_to_pos(int cmd, int pos);
static int  _micron_2_steps_sledge(int micron);
static int	_micron_2_steps_lift(int micron);

//--- sa_init --------------------------------------
void sa_init(void)
{
    motor_config(MOTOR_SLEDGE, CURRENT_HOLD_SLEDGE, STEPS_REV_SLEDGE, DIST_REV_SLEDGE, STEPS);
    motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);

    memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
    _ParSledge.speed = 10000;
    _ParSledge.accel = 32000;
    _ParSledge.current_acc = 420.0;
    _ParSledge.current_run = 300.0;
    _ParSledge.stop_mux = 0;
    _ParSledge.dis_mux_in = 0;
    _ParSledge.encCheck		= chk_off;

    _ParSledgeRef.speed = 1000;
    _ParSledgeRef.accel = 32000;
    _ParSledgeRef.current_acc = 420.0;
    _ParSledgeRef.current_run = 300.0;
    _ParSledgeRef.stop_mux = 0;
    _ParSledgeRef.estop_in_bit[MOTOR_SLEDGE] = 1 << SLEDGE_REF;
    _ParSledgeRef.estop_level = FALSE;
    _ParSledgeRef.dis_mux_in = 0;
    _ParSledgeRef.encCheck = chk_off;

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

    RX_StepperStatus.posX = motor_get_step(MOTOR_SLEDGE);
    RX_StepperStatus.info.headUpInput_0 = !fpga_input(SLEDGE_REF);

    if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.x_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_up    = FALSE;
        RX_StepperStatus.info.z_in_down  = FALSE;
    }
	if (RX_StepperStatus.cmdRunning==0) RX_StepperStatus.info.moving = FALSE;
    if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_ALL_BITS)) 
	{
        RX_StepperStatus.info.headUpInput_0 = !fpga_input(SLEDGE_REF);
		RX_StepperStatus.info.moving = FALSE;
		if (RX_StepperStatus.cmdRunning == CMD_SA_REFERENCE) 
		{
			if (!RX_StepperStatus.info.headUpInput_0) Error(ERR_CONT, 0, "SA: Command REFERENCE: End Sensor NOT HIGH");
			RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0;
            motors_reset(MOTOR_ALL_BITS);				
		}
        else if (motors_error(MOTOR_ALL_BITS, &motor))
		{
			Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);			
			RX_StepperStatus.info.ref_done = FALSE;							
		}

        if (RX_StepperStatus.cmdRunning == CMD_SA_MOVE)
        {
            _SledgePos_Act = _SledgePos_New;
            _SledgePos_New = 0;
        }

        RX_StepperStatus.info.x_in_ref    = (RX_StepperStatus.cmdRunning==CMD_SA_REFERENCE && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_up     = (RX_StepperStatus.cmdRunning==CMD_SA_UP && RX_StepperStatus.info.ref_done);
		RX_StepperStatus.info.z_in_down  = (RX_StepperStatus.cmdRunning==CMD_SA_DOWN && RX_StepperStatus.info.ref_done);
        
		if (RX_StepperStatus.info.ref_done && RX_StepperStatus.cmdRunning==CMD_SA_REFERENCE && _SledgePos_New)
			_sa_move_to_pos(CMD_SA_MOVE,    _SledgePos_New);
		else
			RX_StepperStatus.cmdRunning = FALSE;
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
	term_printf("z in down:     %d\n",	RX_StepperStatus.info.z_in_down);
	term_printf("\n");
}

//--- sa_menu --------------------------------------------------
int sa_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int pos=10000;

	_sa_display_status();
	
	term_printf("LB SA MENU -------------------------\n");
	term_printf("s: STOP\n");
    term_printf("R: Reference\n");
	term_printf("r<n>: reset motor<n>\n");	
    term_printf("p<n>: move sledge to pos <n> um\n");
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
		case 's': sa_handle_ctrl_msg(INVALID_SOCKET, CMD_SA_STOP,			NULL); break;
		case 'R': sa_handle_ctrl_msg(INVALID_SOCKET, CMD_SA_REFERENCE,		NULL); break;
		case 'r': motor_reset(atoi(&str[1]));									   break;
		case 'p': pos = atoi(&str[1]);
                  sa_handle_ctrl_msg(INVALID_SOCKET, CMD_SA_MOVE, &pos);
				  break;
		case 'u': sa_handle_ctrl_msg(INVALID_SOCKET, CMD_SA_UP,		NULL); break;
		case 'd': sa_handle_ctrl_msg(INVALID_SOCKET, CMD_SA_DOWN,		NULL); break;
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
    motors_stop(MOTOR_ALL_BITS);
    motor_config(MOTOR_SLEDGE, CURRENT_HOLD_SLEDGE, STEPS_REV_SLEDGE, DIST_REV_SLEDGE, STEPS);
    motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);
	
	RX_StepperStatus.cmdRunning  = CMD_SA_REFERENCE;
	RX_StepperStatus.info.moving = TRUE;
    motors_move_by_step(1 << MOTOR_SLEDGE, &_ParSledgeRef, -500000, FALSE);
}

//---_micron_2_steps_seldge --------------------------------------------------------------
static int  _micron_2_steps_sledge(int micron)
{
	return (int)(0.5 + STEPS_REV_SLEDGE / DIST_REV_SLEDGE*micron);
}

//---_micron_2_sptes_lift --------------------------------------------------------------
static int _micron_2_steps_lift(int micron)
{
    return (int)(0.5 + STEPS_REV_LIFT / DIST_REV_LIFT * micron);
}

//--- _sa_move_to_pos ---------------------------------------------------------------
static void _sa_move_to_pos(int cmd, int pos)
{
    RX_StepperStatus.cmdRunning  = cmd;
	RX_StepperStatus.info.moving = TRUE;
    motors_move_to_step(1 << MOTOR_SLEDGE, &_ParSledge, pos);
}

//--- sa_handle_ctrl_msg -----------------------------------
int  sa_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	INT32 pos, steps;

	switch(msgId)
	{
    case CMD_SA_STOP:				strcpy(_CmdName, "CMD_SA_STOP");
									motors_stop(MOTOR_ALL_BITS);
									RX_StepperStatus.cmdRunning = 0;
									RX_StepperStatus.info.ref_done = FALSE;
									break;

    case CMD_SA_REFERENCE:			strcpy(_CmdName, "CMD_SA_REFERENCE");
									_SledgePos_New=0;
									_sa_do_reference();	
									break;

    case CMD_SA_MOVE:				strcpy(_CmdName, "CMD_SA_MOVE_SLEDGE");
									pos   = (*((INT32*)pdata));
									steps = _micron_2_steps_sledge(pos);
									if (!RX_StepperStatus.cmdRunning && (steps!=_SledgePos_Act || !RX_StepperStatus.info.ref_done))
									{
                                        _SledgePos_New = steps;
                                        if (RX_StepperStatus.info.ref_done) _sa_move_to_pos(CMD_SA_MOVE, _SledgePos_New);
										else								_sa_do_reference();
									}
									break;

    case CMD_SA_UP:					strcpy(_CmdName, "CMD_SA_UP");
									if (!RX_StepperStatus.cmdRunning && !RX_StepperStatus.info.z_in_up)
									{
                                        RX_StepperStatus.cmdRunning = msgId;
                                        RX_StepperStatus.info.moving = TRUE;
                                        motors_move_to_step(1 << MOTOR_LIFT, &_ParLift, POS_UP);
                                    }
									break;

    case CMD_SA_DOWN:				strcpy(_CmdName, "CMD_SA_DOWN");
									if (!RX_StepperStatus.cmdRunning && !RX_StepperStatus.info.z_in_down)
									{
                                        RX_StepperStatus.cmdRunning = msgId;
                                        RX_StepperStatus.info.moving = TRUE;
                                        motors_move_to_step(1 << MOTOR_LIFT, &_ParLift, -POS_UP);
                                    }
									break;

    case CMD_ERROR_RESET:			fpga_stepper_error_reset();
									break;
                                    
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _sa_motor_z_test ----------------------------------------------------------------------
void _sa_motor_z_test(int steps)
{	
	RX_StepperStatus.cmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_SLEDGE, &_ParSledge, steps, TRUE);
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
    if (motorNo == MOTOR_SLEDGE)
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
    par.estop_in_bit[MOTOR_SLEDGE] = (1 << SLEDGE_REF);
    par.estop_level = FALSE;
	par.dis_mux_in	= 0;
	par.encCheck	= chk_off;
	
	RX_StepperStatus.cmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;

    if (motorNo == MOTOR_SLEDGE)
        motor_config(MOTOR_SLEDGE, CURRENT_HOLD_SLEDGE, STEPS_REV_SLEDGE, DIST_REV_SLEDGE, STEPS);
    else if (motorNo == MOTOR_LIFT)
        motor_config(MOTOR_LIFT, CURRENT_HOLD_LIFT, STEPS_REV_LIFT, DIST_REV_LIFT, STEPS);
    
    motors_move_by_step(motors, &par, steps, FALSE);
}

