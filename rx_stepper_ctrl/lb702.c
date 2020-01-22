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

#define MOTOR_Z_0				0
#define MOTOR_Z_1				1
#define MOTOR_Z_BITS			0x03

#define CURRENT_HOLD	50

#define HEAD_UP_IN_0	0
#define HEAD_UP_IN_1	1

#define MAX_ALIGN		10000	// microns

#define ROBOT_USED_IN		10
#define PRINTHEAD_EN		11	// Input from SPS // '1' Allows Head to go down

#define STEPS_REV		(200*16)	// steps per motor revolution * 16 times oversampling
#define DIST_REV		2000.0	// moving distance per revolution [µm]

#define POS_CAP			RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height + 8000
#define CAL_POS_1		70000
#define CAL_TOOL_HEIGHT	25000

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_calibrate;
static SMovePar	_ParZ_cap;

static int		_CmdRunning=0;
static int		_CmdStep=0;
static int		_NewCmd = 0;

static char		_CmdName[32];
static int		_Cmd_New=0;
static int		_PrintPos_New=0;
static int		_PrintPos_Act=0;
static int		_PrintHeight=0;

static int		_Step;
static int		_CmdRunningRobi = 0;
static int		_Help=FALSE;
static int		_Menu=1;


//--- prototypes --------------------------------------------
static void _lb702_motor_z_test(int steps);
static void _lb702_motor_test  (int motor, int steps);
static void _lb702_do_reference(void);
static void _lb702_move_to_pos(int cmd, int pos);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static int  _incs_2_micron(int incs);

//--- lb702_init --------------------------------------
void lb702_init(void)
{
	RX_StepperStatus.robot_used = fpga_input(ROBOT_USED_IN);
		
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER);
	memset(_CmdName, 0, sizeof(_CmdName));

	//--- movment parameters ----------------
	_ParRef.speed			= 13000;
	_ParRef.accel			= 32000;
	_ParRef.current_acc		= 400.0;
	_ParRef.current_run		= 250.0;
	_ParRef.encCheck		= chk_std;
	_ParRef.enc_bwd			= TRUE;
	_ParRef.stop_mux		= MOTOR_Z_BITS;
	_ParRef.dis_mux_in		= TRUE;
	_ParRef.estop_in_bit[MOTOR_Z_0] = (1<<HEAD_UP_IN_0);
	_ParRef.estop_in_bit[MOTOR_Z_1] = (1<<HEAD_UP_IN_1);
	_ParRef.estop_level		= 1;
	
	_ParZ_down.speed		= 13000;
	_ParZ_down.accel		= 32000;
	_ParZ_down.current_acc	= 400.0;
	_ParZ_down.current_run	= 400.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.encCheck		= chk_std;
	_ParZ_down.enc_bwd		= TRUE;

	_ParZ_calibrate.speed		= 2000;
	_ParZ_calibrate.accel		= 32000;
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
}

//--- lb702_main ------------------------------------------------------------------
void lb702_main(int ticks, int menu)
{
	int motor;
	SStepperStat oldSatus;
	memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));
	
	if (RX_StepperStatus.robot_used) lbrob_main(ticks, menu);
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
	RX_StepperStatus.posZ			    = motor_get_step(MOTOR_Z_0);
	RX_StepperStatus.posY				= motor_get_step(MOTOR_Z_1);
	
	if(RX_StepperCfg.use_printhead_en)
	{
		int enabled=fpga_input(PRINTHEAD_EN);
		if (RX_StepperStatus.info.printhead_en && !enabled) Error(ERR_ABORT, 0, "Allow Head Down signal removed!");
		RX_StepperStatus.info.printhead_en = enabled;	
	}
	else RX_StepperStatus.info.printhead_en = TRUE;
	
	RX_StepperStatus.info.moving = (_CmdRunning!=0);
	if (RX_StepperStatus.info.moving)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;			
	}
	if (_CmdRunning && motors_move_done(MOTOR_Z_BITS))
	{
		RX_StepperStatus.info.moving = FALSE;
        if (_CmdRunning == CMD_CAP_CALIBRATE)
        {            
            rx_sleep(100);
            switch (_CmdStep)
            {
            case 0:	//--- referenced on top ----------------- 
					if (!RX_StepperStatus.info.headUpInput_0) Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor 1 NOT HIGH");
					if (!RX_StepperStatus.info.headUpInput_1) Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor 2 NOT HIGH");
					RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
					motors_reset(MOTOR_Z_BITS);
                    if (RX_StepperStatus.info.ref_done)
                    {                        
                        _CmdStep++;
                        motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, -1*_micron_2_steps(CAL_POS_1));
                    }
                    else 
                    {
                        Error(ERR_CONT, 0, "Calibation Error in step%d", _CmdStep);
						_CmdRunning = FALSE;
					}
                    break;
                   
            case 1:	//--- motor above tool -----------------------------------------
					if (motors_error(MOTOR_Z_BITS, &motor))
					{
						Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked in step %d", _CmdName, motor+1, _CmdStep);			
						RX_StepperStatus.info.ref_done = FALSE;
						_CmdRunning = FALSE;
					}
                    else
                    {
                        _CmdStep++;
                        motors_move_by_step(MOTOR_Z_BITS, &_ParZ_calibrate, -100000, TRUE);
                    }
                    break;
                    
            case 2:	//--- motors blocked at tool -------------------------------------------------
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height = -_incs_2_micron(fpga_encoder_pos(MOTOR_Z_0))+CAL_TOOL_HEIGHT;
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align = -_incs_2_micron(fpga_encoder_pos(MOTOR_Z_1))+CAL_TOOL_HEIGHT - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height;
                    Error(LOG, 0, "Calibration done: ref_height=% 6d, head_align=% 3d", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height, RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align);
                    ctrl_send_2(REP_CAP_CALIBRATE, sizeof(RX_StepperCfg.robot[RX_StepperCfg.boardNo]), &RX_StepperCfg.robot[RX_StepperCfg.boardNo]);
					_CmdRunning = FALSE;
					break;
            }
        }
        else
        {
			if (_CmdRunning == CMD_CAP_REFERENCE) 
			{
				if (!RX_StepperStatus.info.headUpInput_0) Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor 1 NOT HIGH");
				if (!RX_StepperStatus.info.headUpInput_1) Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor 2 NOT HIGH");
				RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
				motors_reset(MOTOR_Z_BITS);				
			}
			else if (motors_error(MOTOR_Z_BITS, &motor))
			{
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);			
				RX_StepperStatus.info.ref_done = FALSE;							
			}
		
			RX_StepperStatus.info.z_in_ref    = ((_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS) && RX_StepperStatus.info.ref_done);
			RX_StepperStatus.info.z_in_print  = (_CmdRunning==CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done && !_CmdRunningRobi);
			RX_StepperStatus.info.z_in_cap    = (_CmdRunning==CMD_CAP_CAPPING_POS);
						
			if (RX_StepperStatus.info.ref_done && _CmdRunning==CMD_CAP_REFERENCE && _Cmd_New && !_CmdRunningRobi)
			{
				_lb702_move_to_pos(_Cmd_New, _PrintPos_New);
				_PrintPos_Act = _PrintPos_New;		
			}
			else 
			{
				_CmdRunning = FALSE;
			}	
        }
		_Cmd_New      = 0;            
	}
	
	if (RX_StepperStatus.robot_used && _CmdRunningRobi)
	{
		int loc_new_cmd = 0;
		if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving) 
		{
			//
			loc_new_cmd = _NewCmd;
			_NewCmd = FALSE;
			_CmdRunningRobi = FALSE;
		}
		if (RX_StepperStatus.robinfo.ref_done && loc_new_cmd)
		{
			switch (loc_new_cmd)
			{
			case CMD_CAP_PRINT_POS: lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, &RX_StepperCfg.print_height); break;
			case CMD_CAP_UP_POS:    lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
			case CMD_CAP_REFERENCE: break;
			default: Error(ERR_CONT, 0, "LB702_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
			}
		}
		loc_new_cmd = FALSE;	
	}
	
	if (memcmp(&oldSatus.info, &RX_StepperStatus.info, sizeof(RX_StepperStatus.info)))
	{
		ctrl_send_2(REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);		
	}
}

//--- _lb702_display_status ---------------------------------------------------------
static void _lb702_display_status(void)
{
	term_printf("LB 702 ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x - %d\n",	RX_StepperStatus.info.moving, _CmdRunning, _CmdStep);
	term_printf("actpos:         %06d  newpos: %06d\n",	_PrintPos_Act, _PrintPos_New);		
	term_printf("refheight:      %06d  ph:     %06d\n", 	_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height), _micron_2_steps(_PrintHeight));
	term_printf("Head UP Sensor: BACK=%d  FRONT=%d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1));
	term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
	term_printf("printhead_en:   %d\n",	RX_StepperStatus.info.printhead_en);
	term_printf("z in reference: %d\n",	RX_StepperStatus.info.z_in_ref);
	term_printf("z in print:     %d\n",	RX_StepperStatus.info.z_in_print);
	term_printf("z in capping:   %d\n",	RX_StepperStatus.info.z_in_cap);
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
		case 's': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,		NULL); break;
		case 'R': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,	NULL); break;
		case 'C': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CALIBRATE,	NULL); break;            
		case 'r': motors_reset(1<<atoi(&str[1])); break;
		case 'c': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS,NULL); break;
		case 'p': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,	&pos); break;
		case 'u': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS,		NULL); break;
		case 'z': _lb702_motor_z_test(atoi(&str[1]));break;
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
	if (RX_StepperStatus.robot_used) lbrob_display_status();
	
	if (_Menu==1) term_printf("LB 702 MENU -------------------------\n");
	else          term_printf("ROBOT MENU --------------------------\n");
	if (_Menu==1)
	{
		if (_Help)
		{
			term_printf("s: STOP\n");
			term_printf("o: toggle output <no>\n");
			term_printf("r<n>: reset motor<n>\n");	
			term_printf("R: Reference\n");
			term_printf("C: Calibrate (first install calibration tool)\n");
			term_printf("c: move to cap\n");
			term_printf("u: move to UP position\n");
			term_printf("p: move to print\n");
			term_printf("z: move by <steps>\n");
			term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
			term_printf("x: exit\n");	
		}
		else
		{
			term_printf("?: help\n");
			term_printf("2: ROBOT menu\n"); 
		}
	}
	else lbrob_menu(_Help);

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
		case 'x': return FALSE;
		default:
			if (_Menu==1) _lb702_handle_menu(str);
			else           lbrob_handle_menu(str);
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
		_lb702_move_to_pos(CMD_CAP_REFERENCE, -500);
	}
	else
	{
		motors_stop	(MOTOR_Z_BITS);
		motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER);
		motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, 500000, TRUE);		
	}
	_CmdRunning  = CMD_CAP_REFERENCE;
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
static void _lb702_move_to_pos(int cmd, int pos)
{
	_CmdRunning  = cmd;
	
	if (RX_StepperStatus.robot_used && !_CmdRunningRobi && (!RX_StepperStatus.robinfo.ref_done || !RX_StepperStatus.info.x_in_ref) && _CmdRunning != CMD_CAP_REFERENCE) 
	{
		_CmdRunningRobi = CMD_CLN_REFERENCE;
		lbrob_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
		_NewCmd = cmd;
	}
	else if(cmd == CMD_CAP_PRINT_POS)
	{
		RX_StepperStatus.info.moving = TRUE;
		motor_move_to_step(MOTOR_Z_0, &_ParZ_down, pos);
		if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align>MAX_ALIGN || RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align<-MAX_ALIGN)
		{
			Error(ERR_CONT, 0, "Head Alignment too large (max=%d mm)", MAX_ALIGN/1000);
			motor_move_to_step(MOTOR_Z_1, &_ParZ_down, pos);				
		}
		else
		{
			if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align>MAX_ALIGN/2 || RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align<-MAX_ALIGN/2)
				Error(WARN, 0, "Head Alignment large");
			motor_move_to_step(MOTOR_Z_1, &_ParZ_down, pos + _micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align));
		}
		motors_start(MOTOR_Z_BITS, TRUE);	
	} 
	else if (abs(pos - motor_get_step(MOTOR_Z_0)) >= 100 && abs(pos - motor_get_step(MOTOR_Z_1)) >= 100)
	{
		motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
	}
	else _CmdRunning = 0;
}

//--- lb702_handle_ctrl_msg -----------------------------------
int  lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{		
	int val;

	switch(msgId)
	{
	case CMD_CAP_STOP:				strcpy(_CmdName, "CMD_CAP_STOP");
									motors_stop(MOTOR_Z_BITS);
									_CmdRunning = 0;
									_Cmd_New = FALSE;
									if (RX_StepperStatus.robot_used) lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL);
									break;	

	case CMD_CAP_REFERENCE:			strcpy(_CmdName, "CMD_CAP_REFERENCE");
									_lb702_do_reference();
									break;

    case CMD_CAP_CALIBRATE:			strcpy(_CmdName, "CMD_CAP_CALIBRATE");
        							if (TRUE) // simulation
                                    {
                                        int ret;
										RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height = 100000;
										RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align = 1;
										Error(LOG, 0, "Calibration done SIMULATION: ref_height=%06d, head_align=%03d", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height, RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align);
										ret=ctrl_send_2(REP_CAP_CALIBRATE, sizeof(RX_StepperCfg.robot[RX_StepperCfg.boardNo]), &RX_StepperCfg.robot[RX_StepperCfg.boardNo]);
										_CmdRunning = FALSE;
									}
									else
                                    {
										_CmdRunning = msgId;
										_CmdStep=0;
										motors_stop	(MOTOR_Z_BITS);
										motors_reset(MOTOR_Z_BITS);
										motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER);
										motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, 500000, TRUE);		
                                    }
									break;
                                       
    case CMD_CAP_PRINT_POS:			if(!RX_StepperStatus.info.printhead_en) return Error(ERR_ABORT, 0, "Allow Head Down signal not set!");
									strcpy(_CmdName, "CMD_CAP_PRINT_POS");
									_PrintHeight   = (*((INT32*)pdata));
									_Step=0;
								//	Error(LOG, 0, "Goto Print: ref=%d, align=%d, printheight=%d", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height, RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align, _PrintHeight);
									if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height < 10000) Error(ERR_ABORT, 0, "Reference Height not defined");
									else
									{
										val = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height - _PrintHeight);
										if (!RX_StepperStatus.info.x_in_ref && RX_StepperStatus.robot_used)
										{
											if (!RX_StepperStatus.info.z_in_ref)
											{
												_Cmd_New = msgId;
												_PrintPos_New = val;
												_lb702_do_reference();
											}
											else
											{
												_CmdRunningRobi = CMD_CLN_REFERENCE;
												lbrob_handle_ctrl_msg(INVALID_SOCKET, _CmdRunningRobi, NULL);
												_NewCmd = msgId;
											}
										}
										else
										{
											if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height < 90000) Error(WARN, 0, "Reference Height small");
											if ((!_CmdRunning || _CmdRunning == CMD_CAP_REFERENCE) && (!RX_StepperStatus.info.ref_done || !RX_StepperStatus.info.z_in_print || val != _PrintPos_Act))
											{
												_Cmd_New      = msgId;
												_PrintPos_New = val;
												if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
												else if (_CmdRunning != CMD_CAP_REFERENCE) _lb702_do_reference();
											}
										}
									}
									break;
		
	case CMD_CAP_UP_POS:			strcpy(_CmdName, "CMD_CAP_UP_POS");
									val = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height - 20000);
									if (!_CmdRunning)
									{
										_PrintPos_New = val;
										if (RX_StepperStatus.info.ref_done) _lb702_move_to_pos(CMD_CAP_UP_POS, _PrintPos_New);
										else								_lb702_do_reference();
									}
									break;

	case CMD_CAP_CAPPING_POS:		strcpy(_CmdName, "CMD_CAP_CAPPING_POS");
									if (!_CmdRunning)
									{
										_CmdRunning  = msgId;
										RX_StepperStatus.info.moving = TRUE;
										if (RX_StepperStatus.robot_used)
											motors_move_to_step	(MOTOR_Z_BITS,  &_ParZ_down,  -1 * _micron_2_steps(RX_StepperCfg.robot[0].ref_height - POS_CAP));
										else motors_move_to_step (MOTOR_Z_BITS,  &_ParZ_down, -1 * _micron_2_steps(RX_StepperCfg.robot[0].ref_height + RX_StepperCfg.robot[0].head_align - POS_CAP));
									}
									break;
				
	case CMD_ERROR_RESET:			fpga_stepper_error_reset();
									break;
	
	case CMD_CAP_VENT:	break;
		
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _lb702_motor_z_test ----------------------------------------------------------------------
void _lb702_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, steps, TRUE);
}

//--- _lb702_motor_test ---------------------------------
static void _lb702_motor_test(int motorNo, int steps)
{
	int motors = 1<<motorNo;
	SMovePar par;

	memset(&par, 0, sizeof(par));
	par.speed		= 5000;
	par.accel		= 32000;
	par.current_acc	= 400.0;
	par.current_run	= 400.0;
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.enc_bwd		= TRUE;
	par.encCheck	= chk_std;
	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	
	motors_config(motors, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER);
	motors_move_by_step(motors, &par, steps, FALSE);			
}