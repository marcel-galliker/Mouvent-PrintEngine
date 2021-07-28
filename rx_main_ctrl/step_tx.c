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
static int					_WashDone		= FALSE;
static int					_FlushPrepared	= FALSE;
static int					_PrepareFlush	= FALSE;
static int					_AutoCapMode	= FALSE;
static int					_RobotDone		= TRUE;

static void _check_wrinkle_detection(void);
static void _steptx_rob_control(void);
static void _empty_cap(void);
static void _steptx_lift_to_clean_wait_pos(void);
static int	_steptx_lift_in_clean_wait_pos(void);


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
        if (_step_socket[1] != INVALID_SOCKET) memcpy(&RX_StepperStatus.robinfo, &_Status[1].robinfo, sizeof(RX_StepperStatus.robinfo));
        RX_StepperStatus.info.uv_on    = enc_is_uv_on();
		RX_StepperStatus.info.uv_ready = enc_is_uv_ready();
		RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();
    }
	if (no==1)
	{
        _empty_cap();
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
    case ctrl_wipe:			sok_send_2(&_step_socket[0], CMD_LIFT_WIPE_POS, 0, NULL); break;
    case ctrl_cap:			sok_send_2(&_step_socket[0], CMD_LIFT_CAPPING_POS, 0, NULL); break;
    case ctrl_wash:			sok_send_2(&_step_socket[0], CMD_LIFT_WASH_POS, 0, NULL); break;
    case ctrl_vacuum:		sok_send_2(&_step_socket[0], CMD_LIFT_VACUUM_POS, 0, NULL); break;
    case ctrl_vacuum_high:	sok_send_2(&_step_socket[0], CMD_LIFT_VACUUM_HIGH_POS, 0, NULL); break;
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
	if (RX_Config.printer.type==printer_test_table)				sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
    else if (_AutoCapMode && RX_StepperStatus.info.z_in_cap)	steptx_set_autocapMode(FALSE);
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
    return _Status[0].info.z_in_print && (_step_socket[1] == INVALID_SOCKET || _Status[1].robinfo.z_in_print);
}

//--- _steptx_lift_to_clean_wait_pos -----------------------------
static void _steptx_lift_to_clean_wait_pos(void)
{
    if (RX_Config.printer.type == printer_test_table)
        sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
    else // TX801/TX802
    {
        INT32 height = RX_Config.stepper.print_height + RX_Config.stepper.material_thickness;
        sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(height), &height);
    }
}

//--- _steptx_lift_in_clean_pos ----------------------
static int _steptx_lift_in_clean_wait_pos(void)
{
    return _Status[0].info.z_in_print;
}

//--- steptx_lift_to_up_pos -----------------------------------
void steptx_lift_to_up_pos(void)
{
    int i = 0;
    for (i = 0; i < SIZEOF(_step_socket); i++)
    {
        sok_send_2(&_step_socket[i], CMD_LIFT_UP_POS, 0, NULL);
    }
}

//--- steptx_lift_in_up_pos --------------
int	 steptx_lift_in_up_pos(void)
{
	return _Status[0].info.z_in_up || _Status[0].info.z_in_ref;
}

//--- steptx_wd_to_up_pos ----------------------------------------------
void steptx_wd_to_up_pos(void)
{
	if (!_Status[1].robinfo.wd_in_up && !_Status[1].robinfo.moving_wd)
		sok_send_2(&_step_socket[1], CMD_LIFT_UP_POS, 0, NULL);
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
    case rob_fct_cap_empty:		return _Status[1].robinfo.rob_in_cap		&& _Status[1].robinfo.moving == FALSE	&& _Status[1].robinfo.ref_done; break;
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
	case ctrl_vacuum:	return _Status[1].robinfo.vacuum_done && !_Status[1].robinfo.moving; break;
	default:			return FALSE; break;
	}	
}

//--- steptx_rob_stop ------------------------------
void steptx_rob_stop(void)
{
	sok_send_2(&_step_socket[1], CMD_ROB_STOP, 0, NULL);
}

//--- steptx_rob_empty_waste ---------------------------------------
/* 
 * If time <= 0, then a defined time on the Stepperboard will used for the pump time. 
 * The pump time can't be decreased , if it gets set a time here and it has already
 * a time left running, it will take the longer time.
 * 
 * */
void steptx_rob_empty_waste(int time)
{
    if (_step_socket[1] != INVALID_SOCKET)
        sok_send_2(&_step_socket[1], CMD_ROB_EMPTY_WASTE, sizeof(time), &time);
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

//--- steptx_rob_cap_for_flush ---------------------------------
void steptx_rob_cap_for_flush(void)
{
    _FlushPrepared = FALSE;
    _PrepareFlush = TRUE;
    steptx_set_robCtrlMode(ctrl_cap);
}

int steptx_rob_cap_flush_prepared(void)
{
    return _FlushPrepared;
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

void steptx_set_autocapMode(int state)
{
	_AutoCapMode = state;
}

EnFluidCtrlMode state_RobotCtrlMode(void)
{
    return _RobotCtrlMode;
}

static void _empty_cap(void)
{
    static int time;
    int waitTime = 20000; // ms

    if ((RX_StepperStatus.info.z_in_print || RX_StepperStatus.info.z_in_up ||
         RX_StepperStatus.info.z_in_ref) &&
        (!RX_StepperStatus.robinfo.ref_done ||
         RX_StepperStatus.robinfo.rob_in_cap) &&
        !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving &&
        !RX_PrinterStatus.door_open && _RobotCtrlMode <= ctrl_print)
    {
        if (!time)	time = rx_get_ticks();
    }
    else
        time = 0;

    if (!_Status[1].robinfo.ref_done && rx_get_ticks() >= time + waitTime && time)
    {
        step_rob_do_reference();
        time = 0;
    }
    else if (_Status[1].robinfo.rob_in_cap && rx_get_ticks() >= time + waitTime && time)
    {
        step_rob_to_wipe_pos(rob_fct_wipe);
        time = 0;
    }
}

//--- steptx_get_robot_done ---------------------------------------------
int steptx_get_robot_done(void)
{
    return _RobotDone;
}

//--- _steptx_rob_control -------------------------------------------------
static void _steptx_rob_control(void)
{
	static int	_printing;		
	EnFluidCtrlMode	old =  _RobotCtrlMode;
	static int _RisingEdge = FALSE;

    if ((_RobotCtrlMode < ctrl_cap || _RobotCtrlMode > ctrl_cap_step6) && _AutoCapMode)
        steptx_set_autocapMode(FALSE);

    switch(_RobotCtrlMode)
	{		
	//--- ctrl_wash --------------------------------------------------------------------------------------
	case ctrl_wash:				_RobotCtrlMode = ctrl_wash_step1;
								_RobotDone = FALSE;
								if (!step_lift_in_up_pos() || !_steptx_lift_in_clean_wait_pos())	_steptx_lift_to_clean_wait_pos();
                                break;
				
	case ctrl_wash_step1:		if ((_steptx_lift_in_clean_wait_pos() || step_lift_in_up_pos()) && !RX_PrinterStatus.scanner_off)
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
			
	case ctrl_wash_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_wash) && (_steptx_lift_in_clean_wait_pos() || step_lift_in_up_pos()))
								{
									_RobotCtrlMode = ctrl_wash_step4;
									step_lift_to_wipe_pos(ctrl_wash);
								}
								else if (!plc_in_wipe_pos() && !RX_PrinterStatus.scanner_off)
								{
									plc_to_wipe_pos();
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
								_RobotDone = FALSE;
								if (!_steptx_lift_in_clean_wait_pos() && !step_lift_in_up_pos())	_steptx_lift_to_clean_wait_pos();
								break;
				
	case ctrl_wipe_step1:		if ((_steptx_lift_in_clean_wait_pos() || step_lift_in_up_pos()) && !RX_PrinterStatus.scanner_off)
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
									_steptx_lift_to_clean_wait_pos();
								}
								break;

	case ctrl_wipe_step6:		if (_steptx_lift_in_clean_wait_pos())
								{
									_RobotCtrlMode = ctrl_vacuum;
								}
								break;
				
	//--- ctrl_vacuum ----------------------------------------------------
	case ctrl_vacuum:			_RobotCtrlMode = ctrl_vacuum_step1;
								_RobotDone = FALSE;
								if (!step_lift_in_up_pos())	_steptx_lift_to_clean_wait_pos();
								break;
				
	case ctrl_vacuum_step1:		if ((step_lift_in_up_pos() || _steptx_lift_in_clean_wait_pos()) && !RX_PrinterStatus.scanner_off)
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
                                    _RobotCtrlMode = ctrl_vacuum_step5;                                        
									step_rob_wipe_start(ctrl_vacuum);
								}
								break;
				
	case ctrl_vacuum_step5:		if (step_rob_wipe_done(ctrl_vacuum))
								{
                                    if (RX_Config.printer.type == printer_TX404 && RX_Config.headsPerColor == 4)
                                    {
										if (plc_in_wipe_pos())
										{
											_RobotCtrlMode = ctrl_vacuum_step8;
											_RisingEdge = FALSE;
											_Status[1].robinfo.vacuum_done = FALSE;
											step_lift_to_wipe_pos(ctrl_vacuum);
										}
										else
											plc_to_wipe_pos();
									}
                                    else
                                    {
                                        _RobotCtrlMode = ctrl_vacuum_step6;
									    _steptx_lift_to_clean_wait_pos();
								    }
                                }
								break;
				
	case ctrl_vacuum_step6:		if (_steptx_lift_in_clean_wait_pos())
								{
									_RobotCtrlMode=ctrl_vacuum_step7;
									step_rob_to_wipe_pos(rob_fct_vacuum_change);
									_RisingEdge = FALSE;
								}
								break;
				
	case ctrl_vacuum_step7:		if (step_rob_in_wipe_pos(rob_fct_vacuum) == FALSE || _RisingEdge)		// rising edge
								{
									_RisingEdge = TRUE;
									if (step_rob_in_wipe_pos(rob_fct_vacuum) && plc_in_wipe_pos())
									{
										_RobotCtrlMode = ctrl_vacuum_step8;
										step_lift_to_wipe_pos(ctrl_vacuum);
										_RisingEdge = FALSE;
									}
									else if (!plc_in_wipe_pos())
										plc_to_wipe_pos();
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
                                    if (RX_Config.printer.type == printer_TX404 && RX_Config.headsPerColor == 4)
                                    {
                                        _RobotCtrlMode = ctrl_vacuum_step13;
                                    }
                                    else
                                    {
									    _RobotCtrlMode = ctrl_vacuum_step10;
									    _steptx_lift_to_clean_wait_pos();
								    }
								}
								break;
		
	case ctrl_vacuum_step10:	if (_steptx_lift_in_clean_wait_pos())
								{
									_RobotCtrlMode = ctrl_vacuum_step11;
									_RisingEdge = FALSE;
									step_rob_to_wipe_pos(rob_fct_vacuum_change);
								}
								break;
		
	case ctrl_vacuum_step11:	if (step_rob_in_wipe_pos(rob_fct_vacuum) == FALSE || _RisingEdge)		// rising edge
								{
									_RisingEdge = TRUE;
									if (step_rob_in_wipe_pos(rob_fct_vacuum) && plc_in_wipe_pos())
									{
										_RobotCtrlMode = ctrl_vacuum_step12;
										_RisingEdge = FALSE;
										step_lift_to_wipe_pos(ctrl_vacuum);
									}
									else if (!plc_in_wipe_pos())
										plc_to_wipe_pos();
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
									if (_printing) _RobotCtrlMode = ctrl_print;
									else		   _RobotCtrlMode = ctrl_off;
                                    _RobotDone = TRUE;
                                }
								break;
				
	//--- ctrl_cap -----------------------------------------------------------------------------
	case ctrl_cap:				_RobotCtrlMode=ctrl_cap_step1;
								_RobotDone = FALSE;
								step_lift_to_up_pos();
								break;
				
	case ctrl_cap_step1:		if (step_lift_in_up_pos() && !RX_PrinterStatus.scanner_off)
								{
                                    if (!step_rob_reference_done()) step_rob_do_reference();

                                    if (!_AutoCapMode)
                                    {
                                        _RobotCtrlMode = ctrl_cap_step2;
                                        plc_to_fill_cap_pos();
                                    }
                                    else
                                    {
                                        _RobotCtrlMode = ctrl_cap_step3;
                                        plc_to_wipe_pos();
                                        step_rob_to_wipe_pos(rob_fct_cap_empty);
                                    }
                                }
								break;	 
				
	case ctrl_cap_step2:		if (plc_in_fill_cap_pos() && step_rob_reference_done()) 
								{
									if (!_Status[1].robinfo.moving) step_rob_to_wipe_pos(rob_fct_cap);
                                    if (!step_rob_in_wipe_pos(rob_fct_cap))
                                    {
                                        _RobotCtrlMode = ctrl_cap_step3;
                                    }
                                }
								break;
    case ctrl_cap_step3:		if ((step_rob_in_wipe_pos(rob_fct_cap) && !_AutoCapMode) || (step_rob_in_wipe_pos(rob_fct_cap_empty) && _AutoCapMode))
								{
                                    if (!_PrepareFlush && !_AutoCapMode)
                                        _RobotCtrlMode = ctrl_cap_step4;
                                    else if (!_PrepareFlush && _AutoCapMode)
                                        _RobotCtrlMode = ctrl_cap_step5;
                                    else
                                        _FlushPrepared = TRUE;
                                }
                                break;

    case ctrl_cap_step4:		if (!RX_PrinterStatus.scanner_off)
								{
                                    _RobotCtrlMode = ctrl_cap_step5;
                                    plc_to_wipe_pos();
                                }
								break;		
			
	case ctrl_cap_step5:		if (plc_in_wipe_pos())
								{
									_RobotCtrlMode=ctrl_cap_step6;
									step_lift_to_wipe_pos(ctrl_cap);
								}
								break;
			
	case ctrl_cap_step6:		if (step_lift_in_wipe_pos(ctrl_cap))
								{
                                    if (_AutoCapMode)	_RobotCtrlMode = ctrl_print;
                                    else				_RobotCtrlMode = ctrl_off;
                                    _RobotDone = TRUE;
                                }
								break;
		
	case ctrl_off:				_RobotCtrlMode = ctrl_off;
								_RobotDone = TRUE;
								undefine_PurgeCtrlMode();
								break;
                                
    case ctrl_print:			_RobotDone = TRUE;
								break;
	default: return;
	}
	if (_RobotCtrlMode!=old)
		fluid_send_ctrlMode(-1, _RobotCtrlMode, TRUE);
}	
