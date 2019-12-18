// ****************************************************************************
//
//	step_lb.c	Standrd Stepper Control		
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_error.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "network.h"
#include "ctrl_msg.h"
#include "print_ctrl.h"
#include "plc_ctrl.h"
#include "enc_ctrl.h"
#include "gui_svr.h"
#include "step_ctrl.h"
#include "step_lb.h"
#include "fluid_ctrl.h"

#define STEPPER_CNT		4

#define WASTE_VALVE_INTERVAL	1000

static void _check_pump(void);

static RX_SOCKET		_step_socket[STEPPER_CNT]={0};

static SStepperStat		_Status[STEPPER_CNT];
static int				_AbortPrinting=FALSE;

static EnFluidCtrlMode	_RobotCtrlMode[STEPPER_CNT] = {ctrl_undef};

static int _PumpState = FALSE;
static int _PumpState_old = FALSE;
static int _TimeStart = 0;
static int _TimeNow = 0;
static int _WasteValveSwitched = 0;

//--- steplb_init ---------------------------------------------------
void steplb_init(int no, RX_SOCKET psocket)
{
	if (no>=0 && no<STEPPER_CNT)
	{
		_step_socket[no] = psocket;
		memset(&_Status[no], 0, sizeof(_Status[no]));
	}
	memset(_Status, 0, sizeof(_Status));
}

//--- _check_pump --------------------------------------------
static void _check_pump(void)
{
	_PumpState = RX_StepperStatus.robinfo.use_waste_pump;
	if (_PumpState != _PumpState_old)
	{
		sok_send_2(&_step_socket[0], CMD_CLN_WASTE_PUMP, sizeof(_PumpState), &_PumpState);
		_PumpState_old = _PumpState;
	}

	if (_PumpState)
	{
		_TimeNow = rx_get_ticks();
	
		if (RX_StepperStatus.robinfo.use_waste_pump)
		{
			int t = (_TimeNow) / WASTE_VALVE_INTERVAL;
	
			if (t > _WasteValveSwitched)
			{
				_WasteValveSwitched = t;
				RX_StepperStatus.waste_valve++;
				
				
				for (int no = 0; no < SIZEOF(_step_socket); no++)
				{
					if (RX_StepperStatus.waste_valve == no * 3 && !_Status[no].robinfo.use_waste_pump) RX_StepperStatus.waste_valve = 3 * (no + 1);
				}
				if (RX_StepperStatus.waste_valve >= SIZEOF(_step_socket) * 3)		RX_StepperStatus.waste_valve = 0;
				for (int no = 0; no < SIZEOF(_step_socket); no++)
				{
					if (RX_StepperStatus.waste_valve == no * 3 && !_Status[no].robinfo.use_waste_pump) RX_StepperStatus.waste_valve = 3 * (no + 1);
				}
				
				for (int no = 0; no < SIZEOF(_step_socket); no++)
				{
					if (_step_socket[no] && _step_socket[no] != INVALID_SOCKET)
					{
						sok_send_2(&_step_socket[no], CMD_CLN_SET_VALVE, sizeof(RX_StepperStatus.waste_valve), &RX_StepperStatus.waste_valve);
					}
				}
			}
		}
	}
	else RX_StepperStatus.waste_valve = 0;
}

//--- steplb_handle_gui_msg------------------------------------------------------------------
int	 steplb_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{	
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && _step_socket[no]!=INVALID_SOCKET)
		{
			switch(cmd)
			{
			case CMD_TT_STOP:
			case CMD_TT_START_REF:
			case CMD_TT_MOVE_TABLE:
			case CMD_TT_MOVE_LOAD:
			case CMD_TT_MOVE_CAP:
			case CMD_TT_MOVE_PURGE:
			case CMD_TT_MOVE_ADJUST:
			case CMD_TT_SCAN_RIGHT:
			case CMD_TT_SCAN_LEFT:
			case CMD_TT_VACUUM:
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;

			case CMD_TT_SCAN:
						{
							STestTableScanPar par;
							par.speed	= 5;
							par.scanCnt = 5;
							par.scanMode= PQ_SCAN_STD;
							par.yStep   = 10000;

							sok_send_2(&_step_socket[no], CMD_TT_SCAN, sizeof(par), &par);
						}
						break;

			//--- cappping ---------------------------------------------------------
			case CMD_CAP_STOP:
			case CMD_CAP_UP_POS:
			case CMD_CAP_CAPPING_POS:
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;

			case CMD_CAP_REFERENCE:
						TrPrintfL(TRUE, "Stepper[%d].CMD_CAP_REFERENCE", no);
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;
		
			case CMD_CAP_PRINT_POS:
						_AbortPrinting=FALSE;
						sok_send_2(&_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
						break;
			}
		}
	}
	return REPLY_OK;
}

//--- steplb_handle_Status ----------------------------------------------------------------------
int steplb_handle_status(int no, SStepperStat *pStatus)
{
	int i;
	int robot_used=FALSE;
	ETestTableInfo info;
	ERobotInfo		robinfo;
	memcpy(&_Status[no], pStatus, sizeof(_Status[no]));

	memset(&info, 0, sizeof(info));
	memset(&robinfo, 0, sizeof(robinfo));
	info.printhead_en	= TRUE;
	info.ref_done		= TRUE;
	info.z_in_ref		= TRUE;
	info.z_in_print		= TRUE;
	info.z_in_cap		= TRUE;
	info.x_in_cap		= TRUE;
	info.x_in_ref		= TRUE;
	robinfo.use_waste_pump = FALSE;
	robinfo.rob_in_cap = TRUE;
	robinfo.ref_done = TRUE;
				
//	TrPrintf(TRUE, "steplb_handle_Status(%d)", no);
	
	for (i=0; i<STEPPER_CNT; i++)
	{
		if (_step_socket[i] && _step_socket[i]!=INVALID_SOCKET)
		{
//			TrPrintf(TRUE, "Stepper[%d]: ref_done=%d moving=%d  z_in_print=%d  z_in_ref=%d", i, _Status[i].info.ref_done, _Status[i].info.moving, _Status[i].info.z_in_print, _Status[i].info.z_in_ref);
			info.ref_done		&= _Status[i].info.ref_done;
			info.printhead_en	&= _Status[i].info.printhead_en;
			info.moving			|= _Status[i].info.moving;
			info.z_in_ref		&= _Status[i].info.z_in_ref;
			info.z_in_print		&= _Status[i].info.z_in_print;
			info.z_in_cap		&= _Status[i].info.z_in_cap;
			info.x_in_cap		&= _Status[i].info.x_in_cap;
			info.x_in_ref		&= _Status[i].info.x_in_ref;
			robot_used			|= _Status[i].robot_used;
			robinfo.use_waste_pump |= _Status[i].robinfo.use_waste_pump;
			robinfo.rob_in_cap &= _Status[i].robinfo.rob_in_cap;
			robinfo.ref_done &= _Status[i].robinfo.ref_done;
			robinfo.moving |= _Status[i].robinfo.moving;
			if (_Status[i].info.moving) 
			{
				RX_StepperStatus.posX = _Status[i].posX;
				RX_StepperStatus.posY = _Status[i].posY;
				RX_StepperStatus.posZ = _Status[i].posZ;
			}
		}
	};
	RX_StepperStatus.robot_used = robot_used;
	
//	TrPrintf(TRUE, "STEPPER: ref_done=%d moving=%d  z_in_print=%d  z_in_ref=%d", info.ref_done, info.moving, info.z_in_print, info.z_in_ref);
	
	/*
	if (RX_StepperStatus.info.printhead_en  != info.printhead_en)	Error(LOG, 0, "Steppers.printhead_en=%d",	info.printhead_en);
	if (RX_StepperStatus.info.z_in_ref		!= info.z_in_ref)		Error(LOG, 0, "Steppers.z_in_ref=%d",		info.z_in_ref);
	if (RX_StepperStatus.info.z_in_print    != info.z_in_print)		Error(LOG, 0, "Steppers.z_in_print=%d",		info.z_in_print);
	*/

	if (!info.moving) 
	{
		RX_StepperStatus.posX = _Status[no].posX;
		RX_StepperStatus.posY = _Status[no].posY;
		RX_StepperStatus.posZ = _Status[no].posZ;
	}
	if (robinfo.use_waste_pump != RX_StepperStatus.robinfo.use_waste_pump)
	{
		int on = robinfo.use_waste_pump;
		sok_send_2(&_step_socket[0], CMD_CLN_WASTE_PUMP, sizeof(on), &on);
		RX_StepperStatus.robinfo.use_waste_pump = robinfo.use_waste_pump;
	}
	
	if (_AbortPrinting && RX_StepperStatus.info.z_in_print) steplb_lift_to_up_pos();
		
	memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));
	memcpy(&RX_StepperStatus.robinfo, &robinfo, sizeof(RX_StepperStatus.robinfo));
	RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();
	RX_StepperStatus.robinfo.rob_in_cap = robinfo.rob_in_cap;
	
	for (int no = 0; no < STEPPER_CNT; no++)
	{
		if (_step_socket[no] && _step_socket[no]!=INVALID_SOCKET) steplb_rob_control(_RobotCtrlMode[no], no);
	}
	
	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
	_check_pump();
	return REPLY_OK;
}

//--- steplb_to_print_pos --------------------------------
int	 steplb_to_print_pos(void)
{
	_AbortPrinting = FALSE;
	for (int no=0; no<SIZEOF(_step_socket); no++)
	{
		sok_send_2(&_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
	}
	return REPLY_OK;									
}

//--- steplb_abort_printing -----------------------------------------
void  steplb_abort_printing(void)
{
	if(RX_StepperStatus.info.z_in_print) steplb_lift_to_up_pos();
	else _AbortPrinting = TRUE;
}

//--- steplb_lift_to_up_pos ---------------------------
void steplb_lift_to_up_pos(void)
{
	for (int no=0; no<SIZEOF(_step_socket); no++)
	{
		sok_send_2(&_step_socket[no], CMD_CAP_UP_POS, 0, NULL);
	}
	_AbortPrinting = FALSE;
}

//--- steplb_lift_is_up --------------
int	 steplb_lift_in_up_pos(void)
{
	return RX_StepperStatus.info.z_in_ref;
}

//--- steplb_lift_to_up_pos_individually -------------------
void steplb_lift_to_up_pos_individually(int no)
{
	sok_send_2(&_step_socket[no], CMD_CAP_UP_POS, 0, NULL);
}

//--- steplb_lift_in_up_pos_individually -------------------
int	 steplb_lift_in_up_pos_individually(int no)
{
	if (no == -1) return steplb_lift_in_up_pos();
	if (_step_socket[no] == INVALID_SOCKET) return TRUE;
	return _Status[no].info.z_in_ref;
}

//--- steplb_rob_to_wipe_pos --------------------------
void steplb_rob_to_wipe_pos(int no, ERobotFunctions rob_function)
{
	sok_send_2(&_step_socket[no], CMD_CLN_MOVE_POS, sizeof(rob_function), &rob_function);		
}

//--- steplb_rob_in_wipe_pos --------------------------
int	 steplb_rob_in_wipe_pos(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return TRUE;
	
	switch (rob_function)
	{
	case rob_fct_cap: 			return _Status[no].robinfo.rob_in_cap		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_wash: 			return _Status[no].robinfo.wash_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_vacuum: 		return _Status[no].robinfo.vacuum_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_wipe: 			return _Status[no].robinfo.wipe_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_vacuum_change: return _Status[no].robinfo.vacuum_in_change	&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_purge_all:		return _Status[no].robinfo.purge_ready		&& _Status[no].robinfo.moving == FALSE; break;
	case rob_fct_purge_head0:
	case rob_fct_purge_head1:
	case rob_fct_purge_head2:
	case rob_fct_purge_head3:
	case rob_fct_purge_head4:
	case rob_fct_purge_head5:
	case rob_fct_purge_head6:
	case rob_fct_purge_head7:	return _Status[no].robinfo.purge_ready		&& _Status[no].robinfo.moving == FALSE; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_in_wipe_pos_all ------------------------
int steplb_rob_in_wipe_pos_all(ERobotFunctions rob_function)
{
	switch (rob_function)
	{
	case rob_fct_cap:	return RX_StepperStatus.robinfo.rob_in_cap	&& RX_StepperStatus.robinfo.moving == FALSE; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_wipe_start -------------------------------------
void steplb_rob_wipe_start(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return;
	sok_send_2(&_step_socket[no], CMD_CLN_FILL_CAP, sizeof(rob_function), &rob_function);
}

//--- steplb_rob_wipe_done --------------------------------------
int	 steplb_rob_wipe_done(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no]==INVALID_SOCKET) return TRUE;
	switch (rob_function)
	{
	case rob_fct_wipe:		return _Status[no].robinfo.wipe_done;
	case rob_fct_wash:		return _Status[no].robinfo.wash_done;
	case rob_fct_vacuum:	return _Status[no].robinfo.vacuum_done;
	case rob_fct_cap:		return _Status[no].robinfo.cap_ready;
	default:				return FALSE;
	}	
}

//--- steplb_rob_stop ------------------------------
void steplb_rob_stop(void)
{
	for (int no = 0; no<SIZEOF(_step_socket); no++)
	{
		sok_send_2(&_step_socket[no], CMD_CLN_STOP, 0, NULL);
		sok_send_2(&_step_socket[no], CMD_CAP_STOP, 0, NULL);
	}	
}

//--- steplb_rob_do_reference -----------------------
void steplb_rob_do_reference(void)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		//if (_step_socket[no] != INVALID_SOCKET)
		sok_send_2(&_step_socket[no], CMD_CLN_REFERENCE, 0, NULL);
	}
}

//--- steplb_rob_reference_done ---------------------
int steplb_rob_reference_done(void)
{
	return RX_StepperStatus.robinfo.ref_done;
}

//--- steplb_lift_to_wipe_pos ----------------------------
void steplb_lift_to_wipe_pos(int no, ERobotFunctions rob_function)
{
	switch (rob_function)
	{
	case rob_fct_cap: sok_send_2(&_step_socket[no], CMD_CAP_CAPPING_POS, 0, NULL); break;
	default: break;
	}
}

//--- steplb_lift_in_wipe_pos -------------------------------
int  steplb_lift_in_wipe_pos(int no, ERobotFunctions rob_function)
{
	if (_step_socket[no] == INVALID_SOCKET) return TRUE;
	switch (rob_function)
	{
	case rob_fct_cap: return _Status[no].info.z_in_cap; break;
	default: return FALSE; break;
	}
}

//--- steplb_rob_control -------------------------------
void steplb_rob_control_all(EnFluidCtrlMode ctrlMode)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] != INVALID_SOCKET)		steplb_rob_control(ctrlMode, no);
	}
}

//--- steplb_rob_control ------------------------------
void steplb_rob_control(EnFluidCtrlMode ctrlMode, int no)
{		
	if (_Status[no].robot_used)
	{
		EnFluidCtrlMode	old = _RobotCtrlMode[no];
		switch (ctrlMode)
		{
		case ctrl_cap:				if (!steplb_rob_reference_done()) steplb_rob_do_reference();
									_RobotCtrlMode[no] = ctrl_cap_step1;
									break;
		
		case ctrl_cap_step1:		if (steplb_rob_reference_done())
									{
										steplb_rob_to_wipe_pos(no, rob_fct_cap);
										_RobotCtrlMode[no] = ctrl_cap_step2;
									}
									break;
		
		case ctrl_cap_step2:		if (steplb_rob_in_wipe_pos_all(rob_fct_cap))
									{
										steplb_rob_wipe_start(no, rob_fct_cap);
										_RobotCtrlMode[no] = ctrl_cap_step3;
									}
									break;
		
		case ctrl_cap_step3:		if (steplb_rob_wipe_done(no, rob_fct_cap))
									{
										steplb_lift_to_wipe_pos(no, rob_fct_cap);
										_RobotCtrlMode[no] = ctrl_cap_step4;
									}
									break;
		
		case ctrl_cap_step4:		if (steplb_lift_in_wipe_pos(no, rob_fct_cap))																_RobotCtrlMode[no] = ctrl_off;
									break;
		
		case ctrl_robi_out:			sok_send_2(&_step_socket[no], CMD_CLN_REFERENCE, 0, NULL);
									_RobotCtrlMode[no] = ctrl_robi_out_step1;
									break;
	
		case ctrl_robi_out_step1:	if (RX_StepperStatus.info.x_in_ref && !RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving)	_RobotCtrlMode[no] = ctrl_off;
									break;

		case ctrl_off:				_RobotCtrlMode[no] = ctrl_off;
									break;
		default: return;
		
		}
		if (_RobotCtrlMode[no] != old)
		{
			fluid_send_ctrlMode(2 * no, _RobotCtrlMode[no], TRUE);
			fluid_send_ctrlMode(2 * no + 1, _RobotCtrlMode[no], TRUE);
		}		
	}
}

