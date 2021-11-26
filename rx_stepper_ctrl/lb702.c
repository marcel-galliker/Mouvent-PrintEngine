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
#include "rx_threads.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "lbrob.h"
#include "lb702.h"
#include "stepper_ctrl.h"
#include "robi_def.h"
#include "robot_client.h"
#include "robi_lb702.h"


#define MOTOR_Z_BACK	0
#define MOTOR_Z_FRONT	1
#define MOTOR_Z_BITS	0x03

#define HEAD_UP_IN_BACK		0
#define HEAD_UP_IN_FRONT	1

static char		*_MotorName[2] = {"BACK", "FRONT"};

#define CURRENT_HOLD			50

#define MAX_ALIGN				10000		// microns

#define SCEWER_USED_IN			 9
#define CLN_USED_IN				10
#define PRINTHEAD_EN			11			// Input from SPS // '1' Allows Head to go down

#define STEPS_REV				(200*STEPS)	// steps per motor revolution * STEPS times oversampling
#define DIST_REV				2000.0		// moving distance per revolution [um]

#define DIST_MECH_REF			500

#define MIN_CAP_HEIGHT			7500		// um under Ref height

#define DIST_CAP_WASH			3400		// um -> higher than capping hight
#define DIST_CAP_SCREW			6100		// um -> higher than capping hight

#define CLUSTER_CHANGE_HEIGHT	60000	//um
#define UP_HEIGHT				20000	//um

static SMovePar	_ParRef;
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_calibrate;
static SMovePar	_ParZ_cap;

static int		_PrintPos_New[2];
static int		_PrintPos_Act[2];
static int		_SlideToRef;
static int		_PrintHeight=0;
static int		_PrintHeightAct = 0;

static UINT32	_ErrorFlags=0;

static int		_Step;
static int		_lift_ref_running=FALSE;
static int		_Help=FALSE;
static int		_Menu=1;
static int		_lift = TRUE;
static int		_rob = TRUE;
static int		_robClient = TRUE;

//--- prototypes --------------------------------------------
static void _lb702_motor_z_test(int steps);
static void _lb702_motor_test  (int motor, int steps);
static void _lb702_move_to_pos(int cmd, int pos0, int pos1, char *file, int line);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static int  _incs_2_micron(int incs);

static void _lb702_start_sm(int cmd, int fromRef, int pos);
static void _lb702_sm(void);

//--- lb702_init --------------------------------------
void lb702_init(void)
{
#ifdef DEBUG
//	RX_StepperStatus.cln_used = (RX_StepperCfg.printerType==printer_LB702_WB);
	RX_StepperStatus.cln_used = fpga_input(CLN_USED_IN);
	RX_StepperStatus.screwer_used	= fpga_input(SCEWER_USED_IN);
#else
	RX_StepperStatus.cln_used		= fpga_input(CLN_USED_IN);
	RX_StepperStatus.screwer_used	= fpga_input(SCEWER_USED_IN);
#endif

    motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);

	//--- movment parameters ----------------
	_ParRef.speed			= 5000;  // Peter defined:  3000; 
	_ParRef.accel			= 15000; // Peter defined: 32000; 
	_ParRef.current_acc		= 400.0;
	_ParRef.current_run		= 250.0;
	_ParRef.encCheck		= chk_std;
	_ParRef.enc_bwd			= TRUE;
	_ParRef.stop_mux		= MOTOR_Z_BITS;
	_ParRef.dis_mux_in		= TRUE;
	_ParRef.estop_in_bit[MOTOR_Z_BACK]  = (1<<HEAD_UP_IN_BACK);
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
		
	lbrob_init(RX_StepperStatus.screwer_used);
        
    RX_StepperStatus.robinfo.auto_cap = TRUE;
}

//--- lb702_main ------------------------------------------------------------------
void lb702_main(int ticks, int menu)
{
	int motor;
	SStepperStat oldSatus;
	memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));
	
	if (RX_StepperStatus.cln_used) lbrob_main(ticks, menu);
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

	if (RX_StepperStatus.info.moving || !RX_StepperStatus.info.ref_done || _lift_ref_running)
	{
		RX_StepperStatus.info.z_in_ref		= FALSE;
		RX_StepperStatus.info.z_in_up		= FALSE;
		RX_StepperStatus.info.z_in_print	= FALSE;
		RX_StepperStatus.info.z_in_cap		= FALSE;			
        RX_StepperStatus.info.z_in_screw	= FALSE;
        RX_StepperStatus.info.z_in_exchange = FALSE;
    }
	if ((RX_StepperStatus.cmdRunning || _lift_ref_running) && motors_move_done(MOTOR_Z_BITS))
	{
        if (_lift_ref_running)
		{
			_lift_ref_running = FALSE;
			if (!RX_StepperStatus.info.ref_done)
			{
                RX_StepperStatus.info.ref_done = RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
				
				if (RX_StepperStatus.info.ref_done)
				{
                    motors_reset(MOTOR_Z_BITS);
                    RX_StepperStatus.info.z_in_ref = TRUE;
				}
				else if (ps_get_power() < 20000)
                {
                    Error(ERR_CONT, 0, "Voltage on Motor power supply too low (%dV)", ps_get_power()/1000);
                    RX_StepperStatus.cmdRunning = FALSE;
                }
                else
				{                    
                    if (!RX_StepperStatus.info.headUpInput_0)
                        Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor BACK NOT HIGH");
                    if (!RX_StepperStatus.info.headUpInput_1)
                        Error(ERR_CONT, 0, "LB702: Command REFERENCE: End Sensor FRONT NOT HIGH");
					RX_StepperStatus.cmdRunning=FALSE;
				}
			}			
			else
			{
				if (motors_error(MOTOR_Z_BITS, &motor))
				{				
                    Error(ERR_CONT, 0, "LIFT: Command %s - 1000 steps: Motor %s blocked", MsgIdStr(RX_StepperStatus.cmdRunning), _MotorName[motor]);
					RX_StepperStatus.info.ref_done = FALSE;
				}
				else RX_StepperStatus.info.z_in_ref = TRUE;
			}
			if (RX_StepperStatus.cmdRunning==CMD_LIFT_REFERENCE)
			{					
				Error(LOG, 0, "CMD_LIFT_REFERENCE done");
				RX_StepperStatus.info.z_in_ref = TRUE;
				RX_StepperStatus.cmdRunning = FALSE;
			}
		}
		else if (motors_error(MOTOR_Z_BITS, &motor))
		{
			Error(ERR_CONT, 0, "LIFT: Command %s: Motor %s blocked", MsgIdStr(RX_StepperStatus.cmdRunning), _MotorName[motor]);			
			RX_StepperStatus.info.ref_done = FALSE;							
            RX_StepperStatus.cmdRunning = FALSE;
		}

		if (RX_StepperStatus.cmdRunning==CMD_LIFT_TEST) RX_StepperStatus.cmdRunning=0;
	}
	
	_lb702_sm();
	
    if (memcmp(&oldSatus.info, &RX_StepperStatus.info, sizeof(RX_StepperStatus.info)))
	{
		ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);		
	}
}

//--- _lb702_display_status ---------------------------------------------------------
static void _lb702_display_status(void)
{
	term_printf("LB 702 ---------------------------------\n");
//	term_printf("actpos0: \t %06d  newpos0:      %06d\n",	_PrintPos_Act[MOTOR_Z_BACK], _PrintPos_New[MOTOR_Z_BACK]);
//	term_printf("actpos1: \t %06d  newpos1:      %06d\n", _PrintPos_Act[MOTOR_Z_FRONT], _PrintPos_New[MOTOR_Z_FRONT]);
    term_printf("actpos front: \t %06d  actpos back:  %06d\n", RX_StepperStatus.posZ, RX_StepperStatus.posZ_back);
    term_printf("Ref Height back(um): \t %06d  Print Height: \t %06d\n", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back,  _PrintHeight);
    term_printf("Ref Height front(um): \t %06d  Print Height: \t %06d\n", RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front, _PrintHeight);
    term_printf("Head UP Sensor: BACK= \t %d \t FRONT= \t %d\n",	fpga_input(HEAD_UP_IN_BACK), fpga_input(HEAD_UP_IN_FRONT));
    term_printf("moving: \t\t %d \t cmd: %s /%d\n",	RX_StepperStatus.info.moving, MsgIdStr(RX_StepperStatus.cmdRunning), _Step);
    term_printf("reference done: \t %d\n",	RX_StepperStatus.info.ref_done);
    term_printf("printhead_en: \t\t %d\n",	RX_StepperStatus.info.printhead_en);
    term_printf("z in reference: \t %d\t",	RX_StepperStatus.info.z_in_ref);
    term_printf("z in up: %d\t",	RX_StepperStatus.info.z_in_up);
    term_printf("z in print: %d\t",	RX_StepperStatus.info.z_in_print);
    term_printf("z in capping: %d\t",	RX_StepperStatus.info.z_in_cap);
    term_printf("z in washing: %d\t", RX_StepperStatus.info.z_in_wash);
    if (RX_StepperCfg.boardNo == 0)
        term_printf("Auto Cap State: %d\n", RX_StepperStatus.robinfo.auto_cap);
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
		case 's': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_STOP,		NULL, _FL_);	break;
		case 'o': Fpga.par->output ^= (1 << atoi(&str[1]));									break;
		case 'R': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_REFERENCE,	NULL, _FL_);	break;           
		case 'r': motors_reset(1<<atoi(&str[1]));											break;
		case 'c': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CAPPING_POS,NULL, _FL_);	break;
        case 'w': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_WASH_POS, NULL, _FL_);		break;
		case 'p': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS,	&pos, _FL_);	break;
        case 'e': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CLUSTER_CHANGE, NULL, _FL_); break;
		case 'u': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS,		NULL, _FL_); break;
        case 'S': lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL, _FL_);		break;
		case 'z': _lb702_motor_z_test(atoi(&str[1]));										break;
        case 'a': RX_StepperStatus.robinfo.auto_cap = !RX_StepperStatus.robinfo.auto_cap;	break;
		case 'm': _lb702_motor_test(str[1]-'0', atoi(&str[2]));								break;
        case 'L': _lift = !_lift;															break;
        case 'C': _rob = !_rob;																break;
        case 'A': _robClient = !_robClient;													break;
		}
	}			
}

//--- lb702_menu --------------------------------------------------
int lb702_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	int pos=10000;

	if (_lift)	_lb702_display_status();
    if (RX_StepperStatus.cln_used)
    {
        if (_rob)		lbrob_display_status();
		if (_robClient)	rc_display_status();
    }

    if (_Menu == 1)
        term_printf("LB 702 MENU -------------------------\n");
    else if (_Menu == 2)
        term_printf("CLEAN MENU --------------------------\n");
    else
        term_printf("ROBOT MENU --------------------------\n");
	
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
            if (_lift)
                term_printf("L: hide LB702 Menu  ");
            else
                term_printf("L: show LB702 Menu  ");
            if (_rob)
                term_printf("C: hide CLEAN Menu  ");
            else
                term_printf("C: show CLEAN Menu  ");
            if (_robClient)
                term_printf("A: hide ROBOT Menu\n");
            else
                term_printf("A: show ROBOT Menu\n");
            term_printf("x: exit\n");
            
		}
		else
		{
			term_printf("?: help\n");
			term_printf("2: CLEAN menu\n"); 
            term_printf("3: ROBOT menu\n");
		}
	}
    else if (_Menu == 2)
        lbrob_menu(_Help);
    else 
		rc_menu(_Help);

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
//		case 'x': return FALSE;
		default:  if (_Menu == 1)
				      _lb702_handle_menu(str);
				  else if (_Menu == 2)
				      lbrob_handle_menu(str);
				  else
						rc_handle_menu(str);
				  break;
		}		
	}
	return TRUE;
}

//--- lb702_do_reference ----------------------------------------------------------------
void lb702_do_reference(void)
{
	if (RX_StepperStatus.info.ref_done)
	{
		TrPrintfL(TRUE, "lb702_do_reference 2");
		_lift_ref_running=TRUE;	
		int pos = -1*_micron_2_steps(DIST_MECH_REF);
		motors_move_to_step	(MOTOR_Z_BITS,  &_ParZ_down, pos);		
	}
	else
	{
		TrPrintfL(TRUE, "lb702_do_reference 3");
		_lift_ref_running=TRUE;	
		motors_stop	(MOTOR_Z_BITS);
		rx_sleep(100);
		motors_reset(MOTOR_Z_BITS);
		motors_config(MOTOR_Z_BITS, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
		motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, 500000, TRUE);		
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
static void _lb702_move_to_pos(int cmd, int pos0, int pos1, char *file, int line)
{
    RX_StepperStatus.cmdRunning = cmd;
	TrPrintfL(TRUE, "_lb702_move_to_pos cmd=0x%08x, pos0=%d, pos1=%d [%s:%d]", cmd, pos0, pos1, file, line);
    if (RX_StepperStatus.cln_used && !RX_StepperStatus.screwerinfo.z_in_down)
    {
		Error(ERR_CONT, 0, "Robot not down!");
    }
    else if (!RX_StepperStatus.info.ref_done)
    {
        _PrintPos_New[MOTOR_Z_BACK] = pos0;
        _PrintPos_New[MOTOR_Z_FRONT] = pos1;
        lb702_do_reference();
    }
    else if (RX_StepperStatus.cln_used && !RX_StepperStatus.screwerinfo.in_garage && RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE && RX_StepperStatus.cmdRunning != CMD_LIFT_CAPPING_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_WASH_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_SCREW)
    {
        RX_StepperStatus.cmdRunning = 0;
        rc_to_garage();
    }
	else if (RX_StepperStatus.cln_used && (!RX_StepperStatus.robinfo.ref_done || !RX_StepperStatus.info.x_in_ref) && RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE && RX_StepperStatus.cmdRunning != CMD_LIFT_CAPPING_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_WASH_POS && RX_StepperStatus.cmdRunning != CMD_LIFT_SCREW) 
	{
        RX_StepperStatus.cmdRunning = 0;
		rc_reference(0);
	}
    else 
	{
		int ok=TRUE;
		if (RX_StepperStatus.cln_used)
		{
			if (cmd == CMD_LIFT_PRINT_POS || cmd == CMD_LIFT_UP_POS || cmd == CMD_LIFT_CLUSTER_CHANGE) 
				ok = RX_StepperStatus.info.x_in_ref; 
            else if (cmd == CMD_LIFT_SCREW)
				ok = !RX_StepperStatus.screwerinfo.moving;
		//	else if (cmd == CMD_LIFT_REFERENCE || cmd == CMD_LIFT_CAPPING_POS || cmd == CMD_LIFT_WASH_POS)
		//		ok = TRUE;
		}
		if (ok)
		{
			RX_StepperStatus.info.moving = TRUE;
			_PrintPos_New[MOTOR_Z_BACK]  = pos0;
			_PrintPos_New[MOTOR_Z_FRONT] = pos1;
			if (motor_move_to_step(MOTOR_Z_BACK, &_ParZ_down,  pos0) &&
				motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_down, pos1))
					motors_start(MOTOR_Z_BITS, TRUE);	
		} 
		else 
		{
			Error(WARN, 0, "Command 0x%08x needs to wait: Robot not in secure position (screwer.in_garage=%d, x_in_ref=%d)", cmd, RX_StepperStatus.screwerinfo.in_garage, RX_StepperStatus.info.x_in_ref);
			RX_StepperStatus.cmdRunning = 0;
		}
	}
}

//--- _lb702_start_sm ------------------------------------
static void _lb702_start_sm(int cmd, int fromRef, int pos)
{
	TrPrintfL(TRUE, "_lb702_start_sm cmd=%s, cmdRunning=%s", MsgIdStr(cmd), MsgIdStr(RX_StepperStatus.cmdRunning));
	if (RX_StepperStatus.cmdRunning)
	{
		if (cmd==RX_StepperStatus.cmdRunning) 
			TrPrintfL(TRUE, "_lb702_start_sm cmd=0x%08x already running", cmd);
        else	
			Error(ERR_CONT, 0, "_lb702_start_sm can't start cmd=0x%08x (cmdRunning=0x%08x)", cmd, RX_StepperStatus.cmdRunning);
		return;
	}
	RX_StepperStatus.cmdRunning = cmd;
	if(RX_StepperStatus.info.printhead_en)
	{
		if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back < 10000) 
			Error(ERR_ABORT, 0, "Reference Height back must be > 10mm");
		else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front < 10000) 
			Error(ERR_ABORT, 0, "Reference Height must be > 10mm");
		else if (abs(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back) > MAX_ALIGN)
			Error(ERR_ABORT, 0, "Reference Height front/back too differents (> 10mm)");
        #ifndef DEBUG
		else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height > 15000 && RX_StepperStatus.cln_used)
			Error(ERR_ABORT, 0, "Capping Height must be < 15mm");
        #endif
		else
		{
			if (fromRef && RX_StepperStatus.cln_used)
			{
				_PrintPos_New[MOTOR_Z_BACK]  = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back  - pos);
				_PrintPos_New[MOTOR_Z_FRONT] = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - pos);
				_SlideToRef = TRUE;
			}
            else if (RX_StepperStatus.cln_used)
			{
				_PrintPos_New[MOTOR_Z_BACK]  = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height - pos);
				_PrintPos_New[MOTOR_Z_FRONT] = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].cap_height - pos);
				_SlideToRef = FALSE;
			}
            else
            {
                _PrintPos_New[MOTOR_Z_BACK]  = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back - pos);
				_PrintPos_New[MOTOR_Z_FRONT] = -1*_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_front - pos);
				_SlideToRef = FALSE;
            }
			_Step=1;
			_lb702_sm();
		}
	}
	else ErrorFlag(ERR_ABORT, &_ErrorFlags, 0x00001, 0, "Allow Head Down signal not set!");
}

//--- _lb702_sm ----------------------------------------------------
static void _lb702_sm(void)
{
	static int time;
	int trace=FALSE;

	if (_Step && rx_get_ticks()>time)
	{
		TrPrintfL(TRUE, "_lb702_sm[%s/%d]", MsgIdStr(RX_StepperStatus.cmdRunning), _Step);
		time=rx_get_ticks()+1000;
		trace=TRUE;
	}

	switch(_Step)
	{
    case 0: break;
    case 1:	TrPrintfL(TRUE, "_lb702_sm[%s/%d]", MsgIdStr(RX_StepperStatus.cmdRunning), _Step);
			_Step++;
			RX_StepperStatus.info.z_in_ref		= FALSE;
			RX_StepperStatus.info.z_in_up		= FALSE;
			RX_StepperStatus.info.z_in_print	= FALSE;
			RX_StepperStatus.info.z_in_cap		= FALSE;
			RX_StepperStatus.info.z_in_screw	= FALSE;
            RX_StepperStatus.info.z_in_exchange = FALSE;
            if (!RX_StepperStatus.info.ref_done 
			|| (_SlideToRef && !RX_StepperStatus.info.x_in_ref)) 
					lb702_do_reference();
			break;

    case 2:	// wait until lift out of danger zone
		//	TrPrintfL(TRUE, "_lb702_sm[%s/%d]: lift.ref_done=%d,  moveDone=%d", MsgIdStr(RX_StepperStatus.cmdRunning), _Step, RX_StepperStatus.info.ref_done, motors_move_done(MOTOR_Z_BITS));
			if (RX_StepperStatus.info.ref_done && motors_move_done(MOTOR_Z_BITS))
			{
				TrPrintfL(TRUE, "_lb702_sm[%s/%d]: lift.ref_done=%d,  posZ=%d", MsgIdStr(RX_StepperStatus.cmdRunning), _Step, RX_StepperStatus.info.ref_done, RX_StepperStatus.posZ);
				_Step++;
				if (RX_StepperStatus.cln_used && _SlideToRef && !RX_StepperStatus.info.x_in_ref)
				{
					int pos=-10000; //_micron_2_steps(RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height_back - UP_HEIGHT + 1000);
					if (RX_StepperStatus.posZ > pos)
					{
						lbrob_reference_slide();
					}
					else 
					{
						Error(ERR_CONT, 0, "_lb702_sm[%s/%d]: posZ=%d", MsgIdStr(RX_StepperStatus.cmdRunning), _Step, RX_StepperStatus.posZ);
						RX_StepperStatus.cmdRunning=0;
						_Step=0;
					}
				}
			}
			break;

    case 3:	// wait until slide in reference
			TrPrintfL(trace, "_lb702_sm[%s/%d]: _SlideToRef=%d, x_in_ref=%d", MsgIdStr(RX_StepperStatus.cmdRunning), _Step, _SlideToRef, RX_StepperStatus.info.x_in_ref);
			if (!RX_StepperStatus.cln_used || !_SlideToRef || RX_StepperStatus.info.x_in_ref)
			{
				TrPrintfL(TRUE, "_lb702_sm[%s/%d]: ", MsgIdStr(RX_StepperStatus.cmdRunning), _Step);
				RX_StepperStatus.info.moving = TRUE;
				if (motor_move_to_step(MOTOR_Z_BACK, &_ParZ_down,  _PrintPos_New[MOTOR_Z_BACK]) &&
					motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_down, _PrintPos_New[MOTOR_Z_FRONT]))
                    {
						motors_start(MOTOR_Z_BITS, TRUE);
                        _PrintHeightAct = _PrintHeight;
                    }
				_Step++;
			}
			break;

    case 4:	// wait until lift in position
			TrPrintfL(trace, "_lb702_sm[%s/%d]: moveDone=%d", MsgIdStr(RX_StepperStatus.cmdRunning), _Step,  motors_move_done(MOTOR_Z_BITS));
			if (motors_move_done(MOTOR_Z_BITS))
			{
				TrPrintfL(TRUE, "_lb702_sm[%s/%d]: DONE cmdRunning=%s", MsgIdStr(RX_StepperStatus.cmdRunning), _Step, MsgIdStr(RX_StepperStatus.cmdRunning));
				Error(LOG, 0, "%s done", MsgIdStr(RX_StepperStatus.cmdRunning));
				switch(RX_StepperStatus.cmdRunning)
				{
                case CMD_LIFT_PRINT_POS:		RX_StepperStatus.info.z_in_print	= TRUE; break;
                case CMD_LIFT_UP_POS:			RX_StepperStatus.info.z_in_up		= TRUE; break;
				case CMD_LIFT_CAPPING_POS:		RX_StepperStatus.info.z_in_cap		= TRUE; break;
                case CMD_LIFT_WASH_POS:			RX_StepperStatus.info.z_in_wash		= TRUE; break;			
                case CMD_LIFT_SCREW:			RX_StepperStatus.info.z_in_screw	= TRUE; break;
                case CMD_LIFT_CLUSTER_CHANGE:	RX_StepperStatus.info.z_in_exchange	= TRUE; break;
                default:	Error(ERR_CONT, 0, "_lb702_sm[%d] cmd=%s not implemented", _Step, MsgIdStr(RX_StepperStatus.cmdRunning));
				}
				_Step=0;
				RX_StepperStatus.cmdRunning = 0;
			}
			break;

    default: _Step=0;
	}
}

//--- lb702_handle_ctrl_msg -----------------------------------
int  lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata, char *file, int line)
{		
	int val0, val1;

	TrPrintfL(TRUE, "lb702_handle_ctrl_msg: msgId=%s [%s:%d]", MsgIdStr(msgId), file, line);

	switch(msgId)
	{
	case CMD_LIFT_STOP:				motors_stop(MOTOR_Z_BITS);
									RX_StepperStatus.cmdRunning = 0;
                                    RX_StepperStatus.info.ref_done = FALSE;
                                    lbrob_stop();
									break;	

	case CMD_LIFT_REFERENCE:		if (RX_StepperStatus.cmdRunning)
                                    {
                                        motors_stop(MOTOR_Z_BITS);
                                        RX_StepperStatus.cmdRunning = 0;
                                        RX_StepperStatus.info.ref_done = 0;
                                    }
									RX_StepperStatus.cmdRunning = CMD_LIFT_REFERENCE;
                                    lb702_do_reference();
									break;
                                       
    case CMD_LIFT_SCREW:            _lb702_start_sm(CMD_LIFT_SCREW, FALSE, DIST_CAP_SCREW);
                                    break;

    case CMD_LIFT_PRINT_POS:		_PrintHeight = (*((INT32*)pdata));
									if (!RX_StepperStatus.info.z_in_print || _PrintHeight != _PrintHeightAct)
										_lb702_start_sm(CMD_LIFT_PRINT_POS, TRUE, _PrintHeight);
									break;
		
	case CMD_LIFT_UP_POS:			_lb702_start_sm(CMD_LIFT_UP_POS, TRUE, UP_HEIGHT);										
									break;

	case CMD_LIFT_CAPPING_POS:		_lb702_start_sm(CMD_LIFT_CAPPING_POS, FALSE, 0);
									break;
				
    case CMD_LIFT_WASH_POS:			_lb702_start_sm(CMD_LIFT_WASH_POS, FALSE, DIST_CAP_WASH);										
                                    break;

    case CMD_LIFT_CLUSTER_CHANGE:   _lb702_start_sm(CMD_LIFT_CLUSTER_CHANGE, TRUE, CLUSTER_CHANGE_HEIGHT);										
									break;

	case CMD_ERROR_RESET:			fpga_stepper_error_reset();
									motor_errors_reset();
                                    rc_clear_error();
                                    _ErrorFlags = 0;
									break;
	case CMD_LIFT_VENT:	break;
		
	case CMD_ROB_STOP:
	case CMD_ROB_REFERENCE:
	case CMD_ROB_MOVE_POS:
	case CMD_ROB_SERVICE:
	case CMD_ROB_VACUUM:
	case CMD_ROB_FILL_CAP:
	case CMD_HEAD_ADJUST:
	case CMD_FIND_ALL_SCREWS:
	case CMD_RESET_ALL_SCREWS:		if (RX_StepperStatus.cln_used)
										Error(ERR_CONT, 0, "LIFT: Command %s not implemented", MsgIdStr(msgId));
									break;
		
	default:						Error(ERR_CONT, 0, "LIFT: Command %s not implemented", MsgIdStr(msgId)); break;
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
    par.encCheck = chk_off;

    if (!RX_StepperStatus.cmdRunning)
    {
		RX_StepperStatus.cmdRunning  = CMD_LIFT_TEST; // TEST 1 motor
        RX_StepperStatus.info.moving = TRUE;
		motors_config(motors, CURRENT_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
}