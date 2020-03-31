// ****************************************************************************
//
//	step_tx.c	Standrd Stepper Control		
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
#include "tcp_ip.h"
#include "network.h"
#include "ctrl_msg.h"
#include "print_ctrl.h"
#include "plc_ctrl.h"
#include "enc_ctrl.h"
#include "fluid_ctrl.h"
#include "gui_svr.h"
#include "step_ctrl.h"
#include "step_tx.h"

#define STEPPER_CNT		2

static RX_SOCKET	_step_socket[STEPPER_CNT];
static SStepperStat	_Status[STEPPER_CNT];

static EnFluidCtrlMode		_RobotCtrlMode = ctrl_undef;
static int					_WashDone   = FALSE;

static void _check_wrinkle_detection(void);
static void _steptx_rob_control(void);


//--- steptx_init ---------------------------------------------------
void steptx_init(int stepperNo, RX_SOCKET psocket)
{
	_step_socket[stepperNo] = psocket;
}

//--- steptx_handle_gui_msg------------------------------------------------------------------
int	 steptx_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{
	switch(cmd)
	{
	//--- cappping ---------------------------------------------------------
	case CMD_LIFT_REFERENCE:
				sok_send_2(&_step_socket[0], cmd, 0, NULL);
				sok_send_2(&_step_socket[1], cmd, 0, NULL);
				step_set_vent(TRUE);
				break;

	case CMD_LIFT_STOP:
	case CMD_LIFT_UP_POS:
				sok_send_2(&_step_socket[0], cmd, 0, NULL);
				sok_send_2(&_step_socket[1], cmd, 0, NULL);
				break;
	case CMD_LIFT_CAPPING_POS:
				sok_send_2(&_step_socket[0], cmd, 0, NULL);
				break;
		
	case CMD_LIFT_PRINT_POS:
				tt_cap_to_print_pos();	
				break;
	}
	return REPLY_OK;
}

//--- steptx_handle_status ----------------------------------------------------------------------
int steptx_handle_status(int no, SStepperStat *pStatus)
{
	memcpy(&_Status[no], pStatus, sizeof(_Status[no]));

	if (no==0)
	{
		memcpy(&RX_StepperStatus, pStatus, sizeof(RX_StepperStatus));
		RX_StepperStatus.info.uv_on    = enc_is_uv_on();
		RX_StepperStatus.info.uv_ready = enc_is_uv_ready();
		RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();		
	}
	if (no==1)
	{
		_steptx_rob_control();
	    _check_wrinkle_detection();
	}
    gui_send_msg_2(0, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return REPLY_OK;
}

//--- steptx_lift_to_wipe_pos----------------------------------
void steptx_lift_to_wipe_pos(EnFluidCtrlMode mode)
{
	switch (mode)
	{
	case ctrl_wipe:			sok_send_2(&_step_socket[0], CMD_LIFT_WIPE_POS,			0, NULL); break;
	case ctrl_cap:			sok_send_2(&_step_socket[0], CMD_LIFT_CAPPING_POS,		0, NULL); break;
	case ctrl_wash:			sok_send_2(&_step_socket[0], CMD_LIFT_WASH_POS,			0, NULL); break;
	case ctrl_vacuum:		sok_send_2(&_step_socket[0], CMD_LIFT_VACUUM_POS,		0, NULL); break;
	case ctrl_vacuum_high:	sok_send_2(&_step_socket[0], CMD_LIFT_VACUUM_HIGH_POS,	0, NULL); break;
	default: break;
	}
		
}

//--- steptx_lift_in_wipe_pos----------------------------------
int steptx_lift_in_wipe_pos(EnFluidCtrlMode mode)
{
	switch (mode)
	{
	case ctrl_wipe:		return _Status[0].robinfo.z_in_wipe; 
	case ctrl_cap:		return _Status[0].info.z_in_cap; 
	case ctrl_wash:		return _Status[0].robinfo.z_in_wash;
	case ctrl_vacuum:	return _Status[0].robinfo.z_in_vacuum; 
	default: return TRUE;
	}
}

//--- steptx_lift_to_print_pos --------------------------------
void steptx_lift_to_print_pos(void)
{
	if (RX_Config.printer.type==printer_test_table) sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
	else // TX801/TX802
	{
		INT32 height = RX_Config.stepper.print_height + RX_Config.stepper.material_thickness;
		sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(height), &height);		
		sok_send_2(&_step_socket[1], CMD_LIFT_PRINT_POS, sizeof(height), &height);		
	}
}

//--- steptx_lift_in_print_pos ----------------------
int steptx_lift_in_print_pos(void)
{
	return _Status[0].info.z_in_print;
}

//--- steptx_lift_to_up_pos -----------------------------------
void steptx_lift_to_up_pos(void)
{
	sok_send_2(&_step_socket[0], CMD_LIFT_UP_POS, 0, NULL);		
}

//--- steptx_lift_in_up_pos --------------
int	 steptx_lift_in_up_pos(void)
{
	return _Status[0].info.z_in_ref;
}

//--- steptx_lift_stop ------------------------------
void steptx_lift_stop(void)
{
	sok_send_2(&_step_socket[0], CMD_LIFT_STOP, 0, NULL);
}

//--- steptx_rob_reference_done ---------------------
int steptx_rob_reference_done(void)
{
	return _Status[1].robinfo.ref_done;
}

//--- steptx_rob_do_reference -----------------------
void steptx_rob_do_reference(void)
{
	sok_send_2(&_step_socket[1], CMD_ROB_REFERENCE, 0, NULL);
}

//--- steptx_rob_to_center_pos ----------------------
void steptx_rob_to_center_pos(void)
{
	int pos = 1;
	sok_send_2(&_step_socket[1], CMD_ROB_SHIFT_LEFT, sizeof(pos), &pos);
}

//--- steptx_rob_to_wipe_pos ------------------------
void steptx_rob_to_wipe_pos(ERobotFunctions rob_function)
{
	if (_step_socket[1]==INVALID_SOCKET) return;
	
	sok_send_2(&_step_socket[1], CMD_ROB_MOVE_POS, sizeof(rob_function), &rob_function);		
}
	
//--- steptx_rob_in_wipe_pos ------------------------
int  steptx_rob_in_wipe_pos(ERobotFunctions rob_function)
{
	if (_step_socket[1]==INVALID_SOCKET) return FALSE;
	
	switch (rob_function)
	{
	case rob_fct_cap: 			return _Status[1].robinfo.cap_ready			&& _Status[1].robinfo.moving == FALSE; break;
	case rob_fct_wash: 			return _Status[1].robinfo.wash_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case rob_fct_vacuum: 		return _Status[1].robinfo.vacuum_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case rob_fct_wipe: 			return _Status[1].robinfo.wipe_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case rob_fct_vacuum_change: return _Status[1].robinfo.vacuum_in_change	&& _Status[1].robinfo.moving == FALSE; break;
	default: return FALSE; break;
	}
}

//--- steptx_rob_wipe_start -------------------------------------
void steptx_rob_wipe_start(EnFluidCtrlMode mode)
{
	if (_step_socket[1]==INVALID_SOCKET) return;
	int function=0;
	switch(mode)
	{
	case ctrl_wipe: 
	case ctrl_wash:
	case ctrl_vacuum:	function = 3; break;
	default: break;
	}
	sok_send_2(&_step_socket[1], CMD_ROB_SHIFT_MOV, sizeof(function), &function);		
}

//--- steptx_rob_wipe_done --------------------------------------
int	 steptx_rob_wipe_done(EnFluidCtrlMode mode)
{
	if (_step_socket[1]==INVALID_SOCKET) return TRUE;
	switch (mode)
	{
	case ctrl_wipe:		return _Status[1].robinfo.wipe_done; break;
	case ctrl_wash:		return _Status[1].robinfo.wash_done; break;
	case ctrl_vacuum:	return _Status[1].robinfo.vacuum_done; break;
	default:			return FALSE; break;
	}	
}

//--- steptx_rob_stop ------------------------------
void steptx_rob_stop(void)
{
	sok_send_2(&_step_socket[1], CMD_ROB_STOP, 0, NULL);
}

//--- void _steptx_rob_vacuum_start ------------------------
void _steptx_rob_vacuum_start(void)
{
	steptx_set_robCtrlMode(ctrl_vacuum);
}
		
//--- steptx_rob_wash_start ---------------------------
void steptx_rob_wash_start(void)
{
	_WashDone = FALSE;
	steptx_set_robCtrlMode(ctrl_wash);					
}

//--- steptx_rob_wash_done ----------------------
int	 steptx_rob_wash_done(void)
{
	return _WashDone;
}

//--- _check_wrinkle_detection -----------------------------------------
static void _check_wrinkle_detection(void)
{
    if (RX_PrinterStatus.printState == ps_printing && _Status[1].robinfo.wrinkle_detected)
    {
        pc_pause_printing(FALSE);
        Error(WARN, 0, "PAUSE because of detected wrinkle!");
    }
}

//--- steptx_set_robCtrlMode -------------------------------------------------
void steptx_set_robCtrlMode(EnFluidCtrlMode ctrlMode)
{
	_RobotCtrlMode = ctrlMode;
	_steptx_rob_control();
}

EnFluidCtrlMode state_RobotCtrlMode(void)
{
    return _RobotCtrlMode;
}

//--- _steptx_rob_control -------------------------------------------------
static void _steptx_rob_control(void)
{
	static int	_printing;		
	EnFluidCtrlMode	old =  _RobotCtrlMode;
	static int _RisingEdge = FALSE;
	
	switch(_RobotCtrlMode)
	{		
	//--- ctrl_wash --------------------------------------------------------------------------------------
	case ctrl_wash:				_RobotCtrlMode = ctrl_wash_step1;
								if (!step_lift_in_up_pos() || !step_lift_in_print_pos())	step_lift_to_print_pos();
								break;
				
	case ctrl_wash_step1:		if (step_lift_in_print_pos() || step_lift_in_up_pos())
								{
									_RobotCtrlMode = ctrl_wash_step2;
									if (!step_rob_reference_done()) step_rob_do_reference();
									plc_to_wipe_pos();
								}
								break;
			
	case ctrl_wash_step2:		if (step_rob_reference_done())
								{
									_RobotCtrlMode = ctrl_wash_step3;
									step_rob_to_wipe_pos(rob_fct_wash);
								}
								break;
			
	case ctrl_wash_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_wash) && (step_lift_in_print_pos() || step_lift_in_up_pos()))
								{
									_RobotCtrlMode = ctrl_wash_step4;
									step_lift_to_wipe_pos(ctrl_wash);
								}
								break;
				
	case ctrl_wash_step4:		if (step_lift_in_wipe_pos(ctrl_wash))
								{
									_RobotCtrlMode = ctrl_wash_step5;
									step_rob_wipe_start(ctrl_wash);
								}
								break;
				
	case ctrl_wash_step5:		if (step_rob_wipe_done(ctrl_wash))
								{
									_RobotCtrlMode = ctrl_wash_step6;
									step_lift_to_up_pos();
								}
								break;
				
	case ctrl_wash_step6:		if (step_lift_in_up_pos())
								{
									_WashDone = TRUE;
								}
								break;
				
	//--- ctrl_wipe -------------------------------------------------------------------------------------
	case ctrl_wipe:				_RobotCtrlMode = ctrl_wipe_step1;
								if (!step_lift_in_print_pos() && !step_lift_in_up_pos())	step_lift_to_print_pos();
								break;
				
	case ctrl_wipe_step1:		if (step_lift_in_print_pos() || step_lift_in_up_pos())
								{
									_RobotCtrlMode=ctrl_wipe_step2;
									if (!step_rob_reference_done()) step_rob_do_reference();
									plc_to_wipe_pos();
								}
								break;
			
	case ctrl_wipe_step2:		if (step_rob_reference_done())
								{
									_RobotCtrlMode=ctrl_wipe_step3;										
									step_rob_to_wipe_pos(rob_fct_wipe);
								}
								break;
				
	case ctrl_wipe_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_wipe))
								{
									_RobotCtrlMode=ctrl_wipe_step4;
									step_lift_to_wipe_pos(ctrl_wipe);
								}
								break;
				
	case ctrl_wipe_step4:		if (step_lift_in_wipe_pos(ctrl_wipe))
								{
									_RobotCtrlMode=ctrl_wipe_step5;
									step_rob_wipe_start(ctrl_wipe);
								}
								break;

	case ctrl_wipe_step5:		if (step_rob_wipe_done(ctrl_wipe))
								{
									_RobotCtrlMode=ctrl_wipe_step6;
									step_lift_to_print_pos();
								}
								break;

	case ctrl_wipe_step6:		if (step_lift_in_print_pos())
								{
									_RobotCtrlMode = ctrl_vacuum;
								}
								break;
				
	//--- ctrl_vacuum ----------------------------------------------------
	case ctrl_vacuum:			if (!step_lift_in_up_pos())	step_lift_to_print_pos();
								_RobotCtrlMode = ctrl_vacuum_step1;
								break;
				
	case ctrl_vacuum_step1:		if (step_lift_in_up_pos() || step_lift_in_print_pos())
								{
									_RobotCtrlMode=ctrl_vacuum_step2;
									if (!step_rob_reference_done())
										step_rob_do_reference();
									plc_to_wipe_pos();
								}
								break;
	case ctrl_vacuum_step2:		if (step_rob_reference_done())
								{
									_RobotCtrlMode=ctrl_vacuum_step3;										
									step_rob_to_wipe_pos(rob_fct_vacuum_all);
								}
								break;
	case ctrl_vacuum_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_vacuum))
								{
									_RobotCtrlMode=ctrl_vacuum_step4;
									step_lift_to_wipe_pos(ctrl_vacuum_high);
								}
								break;
				
	case ctrl_vacuum_step4:		if (step_lift_in_wipe_pos(ctrl_vacuum))
								{
									_RobotCtrlMode=ctrl_vacuum_step5;
									step_rob_wipe_start(ctrl_vacuum);
								}
								break;
				
	case ctrl_vacuum_step5:		if (step_rob_wipe_done(ctrl_vacuum))
								{
									_RobotCtrlMode=ctrl_vacuum_step6;
									step_lift_to_print_pos();
								}
								break;
				
	case ctrl_vacuum_step6:		if (step_lift_in_print_pos())
								{
									_RobotCtrlMode=ctrl_vacuum_step7;
									step_rob_to_wipe_pos(rob_fct_vacuum_change);
									_RisingEdge = FALSE;
								}
								break;
				
	case ctrl_vacuum_step7:		if (step_rob_in_wipe_pos(rob_fct_vacuum) == FALSE || _RisingEdge)		// rising edge
								{
									_RisingEdge = TRUE;
									if (step_rob_in_wipe_pos(rob_fct_vacuum))
									{
										_RobotCtrlMode=ctrl_vacuum_step8;
										step_lift_to_wipe_pos(ctrl_vacuum);
										_RisingEdge = FALSE;
									}
								}
								break;
				
	case ctrl_vacuum_step8:		if (step_lift_in_wipe_pos(ctrl_vacuum))
								{
									_RobotCtrlMode=ctrl_vacuum_step9;
									step_rob_wipe_start(ctrl_vacuum);
								}
								break;
				
	case ctrl_vacuum_step9:		if (step_rob_wipe_done(ctrl_vacuum))
								{
									_RobotCtrlMode = ctrl_vacuum_step10;
									step_lift_to_print_pos();
								}
								break;
		
	case ctrl_vacuum_step10:	if (step_lift_in_print_pos())
								{
									_RobotCtrlMode = ctrl_vacuum_step11;
									_RisingEdge = FALSE;
									step_rob_to_wipe_pos(rob_fct_vacuum_change);
								}
								break;
		
	case ctrl_vacuum_step11:	if (step_rob_in_wipe_pos(rob_fct_vacuum) == FALSE || _RisingEdge)		// rising edge
								{
									_RisingEdge = TRUE;
									if (step_rob_in_wipe_pos(rob_fct_vacuum))
									{
										_RobotCtrlMode = ctrl_vacuum_step12;
										_RisingEdge = FALSE;
										step_lift_to_wipe_pos(ctrl_vacuum);
									}
								}
								break;
		
	case ctrl_vacuum_step12:	if (step_lift_in_wipe_pos(ctrl_vacuum))
								{
									_RobotCtrlMode = ctrl_vacuum_step13;
									step_rob_wipe_start(ctrl_vacuum);
								}								
								break;	
		
	case ctrl_vacuum_step13:	if (step_rob_wipe_done(ctrl_vacuum))
								{
									_RobotCtrlMode=ctrl_vacuum_step14;
									_printing = (RX_PrinterStatus.printState==ps_pause);
									if (_printing) step_lift_to_print_pos();
									else		  step_lift_to_up_pos();
								}
								break;
				
	case ctrl_vacuum_step14:	if ((_printing && step_lift_in_print_pos()) || (!_printing && step_lift_in_up_pos()))
								{
									Error(LOG, 0, "ctrl_vacuum_step10 printState=%d", RX_PrinterStatus.printState);
									if (_printing) _RobotCtrlMode = ctrl_print;
									else		   _RobotCtrlMode = ctrl_off;
								}
								break;
				
	//--- ctrl_cap -----------------------------------------------------------------------------
	case ctrl_cap:				_RobotCtrlMode=ctrl_cap_step1;
								step_lift_to_up_pos();
								break;
				
	case ctrl_cap_step1:		if (step_lift_in_up_pos())
								{
									_RobotCtrlMode=ctrl_cap_step2;
									if (!step_rob_reference_done()) step_rob_do_reference();
									plc_to_fill_cap_pos();
								}
								break;	 
				
	case ctrl_cap_step2:		if (plc_in_fill_cap_pos() && step_rob_reference_done()) 
								{
									step_rob_to_wipe_pos(rob_fct_cap);
									if (!step_rob_in_wipe_pos(rob_fct_cap)) _RobotCtrlMode = ctrl_cap_step3;
								}
								break;
			
	case ctrl_cap_step3:		if (step_rob_in_wipe_pos(rob_fct_cap))
								{
									_RobotCtrlMode=ctrl_cap_step4;
									plc_to_wipe_pos();
								}
								break;		
			
	case ctrl_cap_step4:		if (plc_in_wipe_pos())
								{
									_RobotCtrlMode=ctrl_cap_step5;
									step_lift_to_wipe_pos(ctrl_cap);
								}
								break;
			
	case ctrl_cap_step5:		if (step_lift_in_wipe_pos(ctrl_cap))
								{
									_RobotCtrlMode = ctrl_off;
								}
								break;
		
	case ctrl_off:				_RobotCtrlMode = ctrl_off;
								undefine_PurgeCtrlMode();
                                //Error(LOG, 0, "Program goes though ctrl_off");
								break;
	default: return;
	}
	if (_RobotCtrlMode!=old)
		fluid_send_ctrlMode(-1, _RobotCtrlMode, TRUE);
}	
