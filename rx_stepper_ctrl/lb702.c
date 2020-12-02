// ****************************************************************************
//
//	DIGITAL PRINTING - lb702.c
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
#include "lbrob.h"
#include "lb702.h"
#include "stepper_ctrl.h"
#include "robi_def.h"
#include "robi_lb702.h"


#define MOTOR_Z_BACK	0
#define MOTOR_Z_FRONT	1
#define MOTOR_Z_BITS	0x03

#define HEAD_UP_IN_BACK		0
#define HEAD_UP_IN_FRONT	1

static char		*_MotorName[2] = {"BACK", "FRONT"};

#define CURRENT_HOLD			50

#define MAX_ALIGN				10000		// microns

#define ROBOT_USED_IN			10
#define PRINTHEAD_EN			11			// Input from SPS // '1' Allows Head to go down

#define STEPS_REV				(200*STEPS)	// steps per motor revolution * STEPS times oversampling
#define DIST_REV				2000.0		// moving distance per revolution [�m]

#define DIST_MECH_REF			500

#define MIN_CAP_HEIGHT			7600		// um under Ref height

#define DIST_CAP_WASH			7600		// um -> higher than capping hight
#define DIST_CAP_SCREW			7100		// um -> higher than capping hight

#define CLUSTER_CHANGE_HEIGHT	60000	//um

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_calibrate;
static SMovePar	_ParZ_cap;

static int		_CmdStep=0;
static int		_NewCmd = 0;

static char		_CmdName[32];
static int		_Cmd_New=0;
static int		_PrintPos_New[2];
static int		_PrintPos_Act[2];
static int		_PrintHeight=0;
static int		_TimeOut_Move = 0;

static int		_Step;
static int		_CmdRunningRobi = 0;
static int		_Help=FALSE;
static int		_Menu=1;
static UINT32	_ErrorFlags=0;

//--- prototypes --------------------------------------------
static void _lb702_motor_z_test(int steps);
static void _lb702_motor_test  (int motor, int steps);
static void _lb702_do_reference(void);
static void _lb702_move_to_pos(int cmd, int pos0, int pos1);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static int  _incs_2_micron(int incs);

//--- lb702_init --------------------------------------
void lb702_init(void)
{
	RX_StepperStatus.robot_used = fpga_input(ROBOT_USED_IN);
		
    motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
	memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
	_ParRef.speed			= 5000;  // Peter defined:  3000; 
	_ParRef.accel			= 15000; // Peter defined: 32000; 
	_ParRef.current_acc		= 400.0;
	_ParRef.current_run		= 250.0;
	_ParRef.encCheck		= chk_std;
	_ParRef.enc_bwd			= TRUE;
	_ParRef.stop_mux		= MOTOR_Z_BITS;
	_ParRef.dis_mux_in		= TRUE;
	_ParRef.estop_in_bit[MOTOR_Z_BACK] = (1<<HEAD_UP_IN_BACK);
	_ParRef.estop_in_bit[MOTOR_Z_FRONT] = (1<<HEAD_UP_IN_FRONT);
	_ParRef.estop_level		= 1;
	
	_ParZ_down.speed		= 13000;
	_ParZ_down.accel		= 32000;
	_ParZ_down.current_acc	= 400.0;
	_ParZ_down.current_run	= 400.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.encCheck		= chk_std;
	_ParZ_down.enc_bwd		= TRUE;

	_ParZ_calibrate.speed		= 1000;
	_ParZ_calibrate.accel		= 10000;
	_ParZ_calibrate.current_acc	= 200.0;
	_ParZ_calibrate.current_run	= 100.0;
	_ParZ_calibrate.dis_mux_in	= 0;
	_ParZ_calibrate.encCheck	= chk_std;
	_ParZ_calibrate.enc_bwd		= TRUE;

    _ParZ_cap.speed			= 5000;
	_ParZ_cap.accel			= 32000;
	_ParZ_cap.current_acc	= 200.0;
	_ParZ_cap.current_run	= 200.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= 0;
	_ParZ_cap.encCheck		= chk_std;
	_ParZ_cap.enc_bwd		= TRUE;	
		
	if (RX_StepperStatus.robot_used) lbrob_init();
    
	memcpy(&RX_StepperStatus.screwclusters, &RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters, sizeof(RX_StepperStatus.screwclusters));
    
    memcpy(&RX_StepperStatus.screwpositions, &RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions, sizeof(RX_StepperStatus.screwpositions));

    RX_StepperStatus.robinfo.auto_cap = TRUE;
}

//--- lb702_main ------------------------------------------------------------------
void lb702_main(int ticks, int menu)
{
	int motor;
	SStepperStat oldSatus;
	memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));
	
	if (RX_StepperStatus.robot_used) lbrob_main(ticks, menu);
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_BACK);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_FRONT);
	RX_StepperStatus.posZ			    = motor_get_step(MOTOR_Z_BACK);
	RX_StepperStatus.posZ_back			= motor_get_step(MOTOR_Z_FRONT);
	
	if(RX_StepperCfg.use_printhead_en)
	{
		int enabled=fpga_input(PRINTHEAD_EN);
        if (RX_StepperStatus.info.printhead_en && !enabled)
            Error(ERR_ABORT, 0, "Allow Head Down signal removed!");
		RX_StepperStatus.info.printhead_en = enabled;	
	}
    else
        RX_StepperStatus.info.printhead_en = TRUE;
	
	RX_StepperStatus.info.moving = (RX_StepperStatus.cmdRunning!=0);
	if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_up    = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;			
        RX_StepperStatus.info.z_in_screw = FALSE;
	}
	if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_Z_BITS))
	{
        int cmd = RX_StepperStatus.cmdRunning;
        RX_StepperStatus.info.moving = FALSE;
        if (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE)
		{
			if (!RX_StepperStatus.info.ref_done)
			{
                RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
				
				if (RX_StepperStatus.info.ref_done)
				{
                    motors_reset(MOTOR_Z_BITS);
                    TrPrintfL(TRUE, "CMD_LIFT_REFERENCE done, _Cmd_New=0x%08x", _Cmd_New);
					if (_Cmd_New)
					{
                        RX_StepperStatus.info.z_in_ref = TRUE;
                        _lb702_move_to_pos(_Cmd_New, _PrintPos_New[MOTOR_Z_BACK], _PrintPos_New[MOTOR_Z_FRONT]);
                        memcpy(_PrintPos_Act, _PrintPos_New, sizeof(_PrintPos_Act));
                        RX_StepperStatus.cmdRunning = _Cmd_New;
                        _Cmd_New = FALSE;
                    }
					else
					{
						int pos = -1*_micron_2_steps(DIST_MECH_REF);
						_lb702_move_to_pos(CMD_LIFT_REFERENCE, pos, pos);
                        RX_StepperStatus.cmdRunning = FALSE;
					}
				}
				else 
				{
                    
                    if (!RX_StepperStatus.info.headUpInput_0)
                        Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor BACK NOT HIGH");
                    if (!RX_StepperStatus.info.headUpInput_1)
                        Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor FRONT NOT HIGH");
                    _Cmd_New = FALSE;
					RX_StepperStatus.cmdRunning=FALSE;
				}
			}			
			else
			{
				if (motors_error(MOTOR_Z_BITS, &motor))
				{				
                    Error(ERR_CONT, 0, "LIFT: Command %s - 1000 steps: Motor %s blocked", _CmdName, _MotorName[motor]);
					RX_StepperStatus.info.ref_done = FALSE;
				}
                else if (_Cmd_New)
                {
                    RX_StepperStatus.info.z_in_ref = TRUE;
                    _lb702_move_to_pos(_Cmd_New, _PrintPos_New[MOTOR_Z_BACK], _PrintPos_New[MOTOR_Z_FRONT]);
                    memcpy(_PrintPos_Act, _PrintPos_New, sizeof(_PrintPos_Act));
                    RX_StepperStatus.cmdRunning = _Cmd_New;
                    _Cmd_New = FALSE;
                }
                    
				RX_StepperStatus.cmdRunning = FALSE;
			}
		}
        else if (RX_StepperStatus.cmdRunning == CMD_LIFT_PRINT_POS || RX_StepperStatus.cmdRunning == CMD_LIFT_UP_POS)
		{
			if (motors_error(MOTOR_Z_BITS, &motor))
			{
                Error(ERR_CONT, 0, "LIFT: Command %s: Motor %s blocked", _CmdName, _MotorName[motor]);
				RX_StepperStatus.info.ref_done = FALSE;							
				RX_StepperStatus.cmdRunning = FALSE;
                _NewCmd = FALSE;
            }
			else
                RX_StepperStatus.cmdRunning = FALSE;
        }
        else if (RX_StepperStatus.cmdRunning == CMD_LIFT_CAPPING_POS || RX_StepperStatus.cmdRunning == CMD_LIFT_WASH_POS || RX_StepperStatus.cmdRunning == CMD_LIFT_SCREW)
        {
            if (motors_error(MOTOR_Z_BITS, &motor))
            {
                Error(ERR_CONT, 0, "LIFT: Command %s: Motor %s blocked", _CmdName, _MotorName[motor]);
                RX_StepperStatus.info.ref_done = FALSE;
			}
            RX_StepperStatus.cmdRunning = FALSE;

		}
		else if (motors_error(MOTOR_Z_BITS, &motor))
		{
			Error(ERR_CONT, 0, "LIFT: Command %s: Motor %s blocked", _CmdName, _MotorName[motor]);			
			RX_StepperStatus.info.ref_done = FALSE;							
			RX_StepperStatus.cmdRunning = FALSE;
		}

		if (RX_StepperStatus.cmdRunning==CMD_LIFT_TEST) RX_StepperStatus.cmdRunning=0;
		if (!RX_StepperStatus.cmdRunning)
		{
            RX_StepperStatus.info.z_in_ref = (cmd == CMD_LIFT_REFERENCE && RX_StepperStatus.info.ref_done);
            RX_StepperStatus.info.z_in_up = (cmd == CMD_LIFT_UP_POS && RX_StepperStatus.info.ref_done);
            RX_StepperStatus.info.z_in_print = (cmd == CMD_LIFT_PRINT_POS && RX_StepperStatus.info.ref_done && !_CmdRunningRobi);
            RX_StepperStatus.info.z_in_cap = (cmd == CMD_LIFT_CAPPING_POS && RX_StepperStatus.info.ref_done && RX_StepperStatus.screwerinfo.y_in_ref && RX_RobiStatus.isInGarage);
            RX_StepperStatus.info.z_in_wash = (cmd == CMD_LIFT_WASH_POS && RX_StepperStatus.info.ref_done);
            RX_StepperStatus.info.z_in_screw = (cmd == CMD_LIFT_SCREW && RX_StepperStatus.info.ref_done);
        }
        else
        {
            RX_StepperStatus.cmdRunning = FALSE;
            RX_StepperStatus.info.z_in_ref = FALSE;
            RX_StepperStatus.info.z_in_up = FALSE;
            RX_StepperStatus.info.z_in_print = FALSE;
            RX_StepperStatus.info.z_in_cap = FALSE;
            RX_StepperStatus.info.z_in_wash = FALSE;
		}
	}
	
	if (RX_StepperStatus.robot_used && _CmdRunningRobi)
	{
		int loc_new_cmd = 0;
        if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving && !RX_StepperStatus.screwerinfo.moving)
		{
			//
			loc_new_cmd = _NewCmd;
			_NewCmd = FALSE;
			_CmdRunningRobi = FALSE;
		}
        if (loc_new_cmd == CMD_LIFT_WASH_POS)
        {
            Error(WARN, 0, "CMD_LIFT_WASH_POS is called");
        }
        if (loc_new_cmd)
		{
			switch (loc_new_cmd)
			{
			case CMD_LIFT_PRINT_POS:		lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, &RX_StepperCfg.print_height); break;
			case CMD_LIFT_UP_POS:			lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL); break;
            case CMD_LIFT_CLUSTER_CHANGE:   lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CLUSTER_CHANGE, NULL); break;
            case CMD_LIFT_SCREW:            lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL); break;
            case CMD_LIFT_CAPPING_POS:		lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CAPPING_POS, NULL); break;
            case CMD_LIFT_WASH_POS:			lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_WASH_POS, NULL); break;
			case CMD_LIFT_REFERENCE: break;
			default: Error(ERR_CONT, 0, "LB702_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
			}
		}
		loc_new_cmd = FALSE;
	}
	
    if (memcmp(&oldSatus.info, &RX_StepperStatus.info, sizeof(RX_StepperStatus.info)))
	{
		ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);		
	}
}

//--- _lb702_display_status ---------------------------------------------------------
static void _lb702_display_status(void)
{
	term_printf("LB 702 ---------------------------------\n");
	term_printf("actpos0: \t %06d  newpos0:      %06d\n",	_PrintPos_Act[MOTOR_Z_BACK], _PrintPos_New[MOTOR_Z_BACK]);
	term_printf("actpos1: \t %06d  newpos1:      %06d\n", _PrintPos_Act[MOTOR_Z_FRONT], _PrintPos_New[MOTOR_Z_FRONT]);
    term_printf("actpos front: \t %06d  actpos back:  %06d\n", RX_StepperStatus.posZ, RX_StepperStatus.posZ_back);
    term_printf("Ref Height back(um): \t %06d  Print Height: \t %06d\n", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back,  _PrintHeight);
    term_printf("Ref Height front(um): \t %06d  Print Height: \t %06d\n", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front, _PrintHeight);
    term_printf("Head UP Sensor: BACK= \t %d \t FRONT= \t %d\n",	fpga_input(HEAD_UP_IN_BACK), fpga_input(HEAD_UP_IN_FRONT));
    term_printf("moving: \t\t %d \t cmd: %08x - %d\n",	RX_StepperStatus.info.moving, RX_StepperStatus.cmdRunning, _CmdStep);
    term_printf("reference done: \t %d\n",	RX_StepperStatus.info.ref_done);
    term_printf("printhead_en: \t\t %d\n",	RX_StepperStatus.info.printhead_en);
    term_printf("z in reference: \t %d\n",	RX_StepperStatus.info.z_in_ref);
    term_printf("z in up: \t\t %d\n",	RX_StepperStatus.info.z_in_up);
    term_printf("z in print: \t\t %d\n",	RX_StepperStatus.info.z_in_print);
    term_printf("z in capping: \t\t %d\n",	RX_StepperStatus.info.z_in_cap);
    term_printf("z in washing: \t\t %d\n", RX_StepperStatus.info.z_in_wash);
    if (RX_StepperCfg.boardNo == 0)
        term_printf("Auto Cap State: \t %d\n", RX_StepperStatus.robinfo.auto_cap);
    term_printf("\n");
}

//--- _lb702_handle_menu -----------------------------------------------------
static void _lb702_handle_menu(char *str)
{
	int pos=10000;
	if (str)
	{
		switch (str[0])
		{
		case 's': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_STOP,		NULL); break;
		case 'D': RX_StepperStatus.info.ref_done = FALSE; break;
		case 'R': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_REFERENCE,	NULL); break;           
		case 'r': motors_reset(1<<atoi(&str[1])); break;
		case 'c': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CAPPING_POS,NULL); break;
        case 'w': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_WASH_POS, NULL); break;
		case 'p': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS,	&pos); break;
        case 'e': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CLUSTER_CHANGE, NULL); break;
		case 'u': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS,		NULL); break;
        case 'S': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL); break;
		case 'z': _lb702_motor_z_test(atoi(&str[1]));break;
        case 'a': RX_StepperStatus.robinfo.auto_cap = !RX_StepperStatus.robinfo.auto_cap;
		case 'm': _lb702_motor_test(str[1]-'0', atoi(&str[2]));break;
		}
	}			
}

//--- lb702_menu --------------------------------------------------
int lb702_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	int pos=10000;

	_lb702_display_status();
    if (RX_StepperStatus.robot_used)
    {
        lbrob_display_status();
        robi_lb702_display_status();
    }

    if (_Menu == 1)
        term_printf("LB 702 MENU -------------------------\n");
    else if (_Menu == 2)
        term_printf("ROBOT MENU --------------------------\n");
    else
        term_printf("ROBI MENU --------------------------\n");
	
	if (_Menu==1)
	{
		if (_Help)
		{
			term_printf("s: STOP\n");
			term_printf("o: toggle output <no>\n");
			term_printf("r<n>: reset motor<n>\n");	
			term_printf("R: Reference\n");
			term_printf("c: move to cap\n");
            term_printf("w: move to wash\n");
			term_printf("u: move to UP position\n");
			term_printf("p: move to print\n");
            term_printf("e: move to exchange cluster position\n");
			term_printf("z: move by <steps>\n");
			term_printf("m<n><steps>: move Motor<n> by <steps>\n");
            if (RX_StepperCfg.boardNo == 0)
                term_printf("a: Change Cap Auto State\n");
            term_printf("x: exit\n");	
            
		}
		else
		{
			term_printf("?: help\n");
			term_printf("2: ROBOT menu\n"); 
            term_printf("3: ROBI menu\n");
		}
	}
    else if (_Menu == 2)
        lbrob_menu(_Help);
    else
        robi_lb702_menu(_Help);

	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		_Help = FALSE;
		switch (str[0])
		{
		case '?': _Help=TRUE; break;
		case '1': _Menu = 1; break;
		case '2': _Menu = 2; break;
		case '3': _Menu = 3; break;
		case 'x': return FALSE;
		default:  if (_Menu == 1)
				      _lb702_handle_menu(str);
				  else if (_Menu == 2)
				      lbrob_handle_menu(str);
				  else
				      robi_lb702_handle_menu(str);
				  break;
		}		
	}
	return TRUE;
}

//--- _lb702_do_reference ----------------------------------------------------------------
static void _lb702_do_reference(void)
{
	if (RX_StepperStatus.info.ref_done)
	{
		int pos = -1*_micron_2_steps(DIST_MECH_REF);
		_lb702_move_to_pos(CMD_LIFT_REFERENCE, pos, pos);
	}
	else
	{
		motors_stop	(MOTOR_Z_BITS);
		rx_sleep(100);
		motors_reset(MOTOR_Z_BITS);
		motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
		motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, 500000, TRUE);		
		RX_StepperStatus.cmdRunning  = CMD_LIFT_REFERENCE;
	}
	
	RX_StepperStatus.info.moving = TRUE;
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5+STEPS_REV/DIST_REV*micron);
}

//---_steps_2_micron --------------------------------------------------------------
static int  _steps_2_micron(int steps)
{
	return (int)(0.5+(double)steps/STEPS_REV*DIST_REV);
}

//---_incs_2_micron --------------------------------------------------------------
static int  _incs_2_micron(int incs)
{
	return (int)(0.5+(double)incs/L5918_INC_PER_METER*DIST_REV);
}

//--- _lb702_move_to_pos ---------------------------------------------------------------
static void _lb702_move_to_pos(int cmd, int pos0, int pos1)
{
    RX_StepperStatus.cmdRunning = cmd;
    if (cmd == _NewCmd) _NewCmd = 0;
    if (RX_StepperStatus.robot_used && !_CmdRunningRobi && (!RX_StepperStatus.screwerinfo.y_in_ref || !RX_RobiStatus.isInGarage) && RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE && RX_StepperStatus.cmdRunning != CMD_LIFT_SCREW)
    {
        _CmdRunningRobi = CMD_ROBI_MOVE_TO_GARAGE;
        _NewCmd = cmd;
        RX_StepperStatus.cmdRunning = 0;
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
        
    }
	else if (RX_StepperStatus.robot_used && !_CmdRunningRobi && (!RX_StepperStatus.robinfo.ref_done || !RX_StepperStatus.info.x_in_ref) && RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE && RX_StepperStatus.cmdRunning != CMD_LIFT_CAPPING_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_WASH_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_SCREW) 
	{
		_CmdRunningRobi = CMD_ROB_REFERENCE;
        RX_StepperStatus.cmdRunning = 0;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
		_NewCmd = cmd;
	}
    else if (((cmd == CMD_LIFT_PRINT_POS || cmd == CMD_LIFT_UP_POS || cmd == CMD_LIFT_CLUSTER_CHANGE) && RX_RobiStatus.isInGarage && RX_StepperStatus.screwerinfo.y_in_ref && RX_StepperStatus.info.x_in_ref) ||
                 ((cmd == CMD_LIFT_CAPPING_POS || cmd == CMD_LIFT_WASH_POS) && RX_RobiStatus.isInGarage && RX_StepperStatus.screwerinfo.y_in_ref) || cmd == CMD_LIFT_SCREW || cmd == CMD_LIFT_REFERENCE)
	{
		Error(LOG, 0, "Move Command %08x", cmd);
        RX_StepperStatus.info.moving = TRUE;
		_PrintPos_New[MOTOR_Z_BACK]  = pos0;
		_PrintPos_New[MOTOR_Z_FRONT] = pos1;
		motor_move_to_step(MOTOR_Z_BACK, &_ParZ_down,  pos0);
		motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_down, pos1);

		motors_start(MOTOR_Z_BITS, TRUE);	
	} 
	else 
    {
        Error(WARN, 0, "Command %08x needs to wait", cmd);
        RX_StepperStatus.cmdRunning = 0;
        _NewCmd = cmd;
    }
}

//--- lb702_handle_ctrl_msg -----------------------------------
int  lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{		
	int val0, val1;

	switch(msgId)
	{
	case CMD_LIFT_STOP:				strcpy(_CmdName, "CMD_LIFT_STOP");
									motors_stop(MOTOR_Z_BITS);
									RX_StepperStatus.cmdRunning = 0;
									_Cmd_New = FALSE;
                                    _NewCmd = FALSE;
                                    RX_StepperStatus.info.ref_done = FALSE;
                                    if (RX_StepperStatus.robot_used) lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL);
									break;	

	case CMD_LIFT_REFERENCE:		TrPrintfL(TRUE, "CMD_LIFT_REFERENCE");
									strcpy(_CmdName, "CMD_LIFT_REFERENCE");
                                    if (RX_StepperStatus.cmdRunning)
                                    {
                                        motors_stop(MOTOR_Z_BITS);
                                        RX_StepperStatus.cmdRunning = 0;
                                        RX_StepperStatus.info.ref_done = 0;
                                        _NewCmd = 0;
                                        _Cmd_New = FALSE;
                                    }
                                    _lb702_do_reference();
									break;
                                       
    case CMD_LIFT_SCREW:            TrPrintfL(TRUE, "CMD_LIFT_SCREW");
									strcpy(_CmdName, "CMD_LIFT_SCREW");
									if		(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back  < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height front must be > 10mm");
									else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN) 
										Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");		
									else if (!RX_StepperStatus.cmdRunning)
                                    {
										RX_StepperStatus.cmdRunning  = msgId;
										if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height < MIN_CAP_HEIGHT) 
                                            Error(WARN, 0, "Capping Height should be >= %d.%dmm", MIN_CAP_HEIGHT/1000, (MIN_CAP_HEIGHT%1000)/100);
										val0 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height - DIST_CAP_SCREW);
										val1 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height - DIST_CAP_SCREW);
                                        if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_LIFT_SCREW, val0, val1);
                                    }
                                    break;

    case CMD_LIFT_PRINT_POS:		TrPrintfL(TRUE, "CMD_LIFT_PRINT_POS");
									strcpy(_CmdName, "CMD_LIFT_PRINT_POS");
									if(!RX_StepperStatus.info.printhead_en) return ErrorFlag(ERR_ABORT, &_ErrorFlags, 0x00001, 0, "Allow Head Down signal not set!");
									_PrintHeight   = (*((INT32*)pdata));
									_Step=0;
									if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height must be > 10mm");
									else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN) 
										Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");									
									else if(!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning==CMD_LIFT_REFERENCE)
									{
										val0 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back  - _PrintHeight);
										val1 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - _PrintHeight);
                                        if (!(RX_StepperStatus.screwerinfo.y_in_ref || !RX_RobiStatus.isInGarage) && RX_StepperStatus.robot_used)
                                        {
                                            if (!RX_StepperStatus.info.z_in_ref || RX_StepperStatus.cmdRunning==CMD_LIFT_REFERENCE)
                                            {
                                                _Cmd_New = msgId;
												_PrintPos_New[MOTOR_Z_BACK]  = val0;
												_PrintPos_New[MOTOR_Z_FRONT] = val1;
												if (RX_StepperStatus.cmdRunning!=CMD_LIFT_REFERENCE) _lb702_do_reference();
                                            }
                                            else
                                            {
                                                _CmdRunningRobi = CMD_ROBI_MOVE_TO_GARAGE;
                                                _NewCmd = msgId;
                                                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
                                                
                                            }
                                        }
                                        else if (!RX_StepperStatus.info.x_in_ref && RX_StepperStatus.robot_used)
										{
											if (!RX_StepperStatus.info.z_in_ref || RX_StepperStatus.cmdRunning==CMD_LIFT_REFERENCE)
											{
                                                _Cmd_New = msgId;
												_PrintPos_New[MOTOR_Z_BACK]  = val0;
												_PrintPos_New[MOTOR_Z_FRONT] = val1;
												if (RX_StepperStatus.cmdRunning!=CMD_LIFT_REFERENCE) _lb702_do_reference();
											}
											else
											{
												_CmdRunningRobi = CMD_ROB_REFERENCE;
												lbrob_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
												_NewCmd = msgId;
											}
										}
										else
										{
											if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 90000) Error(WARN, 0, "Reference Height back small");
											if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 90000) Error(WARN, 0, "Reference Height front small");
											if ((!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE) && (!RX_StepperStatus.info.ref_done || !RX_StepperStatus.info.z_in_print || val0 != _PrintPos_Act[MOTOR_Z_BACK] || val1 != _PrintPos_Act[MOTOR_Z_FRONT]))
											{
												TrPrintf(TRUE, "Start REF, _Cmd_New=0x%08x", _Cmd_New);
												_PrintPos_New[MOTOR_Z_BACK]  = val0;
												_PrintPos_New[MOTOR_Z_FRONT] = val1;
												if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_LIFT_PRINT_POS, _PrintPos_New[MOTOR_Z_BACK], _PrintPos_New[MOTOR_Z_FRONT]);
												else if (RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE) 
                                                {
                                                    _Cmd_New = msgId;
                                                    _lb702_do_reference();
                                                }
											}
										}
									}
									break;
		
	case CMD_LIFT_UP_POS:			TrPrintfL(TRUE, "CMD_LIFT_UP_POS");
									strcpy(_CmdName, "CMD_LIFT_UP_POS");
									if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height must be > 10mm");
									else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN) 
										Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");									
									else if (!RX_StepperStatus.cmdRunning)
									{
										val0 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back - 20000);
										val1 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - 20000);
										if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_LIFT_UP_POS, val0, val1);
                                        else							
                                        {
                                            _Cmd_New = msgId;
                                            _lb702_do_reference();
                                        }
									}
									break;

	case CMD_LIFT_CAPPING_POS:		TrPrintfL(TRUE, "CMD_LIFT_CAPPING_POS");
									strcpy(_CmdName, "CMD_LIFT_CAPPING_POS");
									if		(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back  < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height front must be > 10mm");
									else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN) 
										Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");		
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height > 15000) Error(ERR_ABORT, 0, "Capping Height must be < 15mm");
									else if (!RX_StepperStatus.cmdRunning)
									{
										if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height < MIN_CAP_HEIGHT) 
                                            Error(WARN, 0, "Capping Height should be >= %d.%dmm", MIN_CAP_HEIGHT/1000, (MIN_CAP_HEIGHT%1000)/100);
										val0 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height);
										val1 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height);
										if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_LIFT_CAPPING_POS, val0, val1);
									}
									break;
				
    case CMD_LIFT_WASH_POS:			TrPrintfL(TRUE, "CMD_LIFT_WASH_POS");
                                    strcpy(_CmdName, "CMD_LIFT_WASH_POS");
                                    if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
                                    else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height front must be > 10mm");
                                    else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN)
                                        Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");
                                    else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height > 15000)
                                        Error(ERR_ABORT, 0, "Capping Height must be < 15mm");
                                    else if (!RX_StepperStatus.cmdRunning)
                                    {
                                        RX_StepperStatus.cmdRunning = msgId;
                                        if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height < MIN_CAP_HEIGHT)
                                            Error(WARN, 0, "Capping Height should be >= %d.%dmm", MIN_CAP_HEIGHT/1000, (MIN_CAP_HEIGHT%1000)/100);
                                        val0 = -1 * (_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height) - DIST_CAP_WASH);
                                        val1 = -1 * (_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height) - DIST_CAP_WASH);
                                        if (RX_StepperStatus.info.ref_done)
                                            _lb702_move_to_pos(CMD_LIFT_WASH_POS, val0, val1);
                                    }
                                    break;

    case CMD_LIFT_CLUSTER_CHANGE:   TrPrintfL(TRUE, "CMD_LIFT_CLUSTER_CHANGE");
									strcpy(_CmdName, "CMD_LIFT_CLUSTER_CHANGE");
									if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 10000) Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
									else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) Error(ERR_ABORT, 0, "Reference Height must be > 10mm");
									else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN) 
										Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");									
									else if (!RX_StepperStatus.cmdRunning)
                                    {
										val0 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back - CLUSTER_CHANGE_HEIGHT);
										val1 = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - CLUSTER_CHANGE_HEIGHT);
										if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_LIFT_CLUSTER_CHANGE, val0, val1);
                                        else if (RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE)								
                                        {
                                            _Cmd_New = msgId;
                                            _lb702_do_reference();
                                        }
                                    }
									break;

	case CMD_ERROR_RESET:			fpga_stepper_error_reset();
									motor_errors_reset();
									_ErrorFlags = 0;
									break;
	
	case CMD_LIFT_VENT:	break;
		
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _lb702_motor_z_test ----------------------------------------------------------------------
void _lb702_motor_z_test(int steps)
{	
	if (!RX_StepperStatus.cmdRunning)
	{
		RX_StepperStatus.cmdRunning = CMD_LIFT_TEST; // TEST both motors
		RX_StepperStatus.info.moving = TRUE;
		_PrintPos_New[MOTOR_Z_BACK]  = Fpga.encoder[MOTOR_Z_BACK]._pos_motor * STEPS + steps;
		_PrintPos_New[MOTOR_Z_FRONT] = Fpga.encoder[MOTOR_Z_FRONT]._pos_motor * STEPS + steps;
		motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, steps, TRUE);
	}
}

//--- _lb702_motor_test ---------------------------------
static void _lb702_motor_test(int motorNo, int steps)
{
    int motors = 1 << motorNo;
    SMovePar par;

    memset(&par, 0, sizeof(par));
    par.speed = 5000;
    par.accel = 32000;
    par.current_acc = 400.0;
    par.current_run = 400.0;
    par.stop_mux = 0;
    par.dis_mux_in = 0;
    par.enc_bwd = TRUE;
    par.encCheck = chk_std;

    if (!RX_StepperStatus.cmdRunning)
    {
		RX_StepperStatus.cmdRunning  = CMD_LIFT_TEST; // TEST 1 motor
        RX_StepperStatus.info.moving = TRUE;

        motors_config(motors, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
}

//--- lb702_reset_variables ----------------------------------------------------------
void lb702_reset_variables(void)
{
    _NewCmd = FALSE;
}