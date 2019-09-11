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
#include "gui_svr.h"
#include "step_ctrl.h"

#define STEPPER_CNT		2

static RX_SOCKET	_step_socket[STEPPER_CNT];
static SStepperStat	_Status[STEPPER_CNT];


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
	case CMD_CAP_REFERENCE:
				sok_send_2(&_step_socket[0], cmd, 0, NULL);
				step_set_vent(TRUE);
				break;

	case CMD_CAP_STOP:
	case CMD_CAP_UP_POS:
	case CMD_CAP_CAPPING_POS:
				sok_send_2(&_step_socket[0], cmd, 0, NULL);
				break;
		
	case CMD_CAP_PRINT_POS:
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
	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return REPLY_OK;
}

//--- steptx_lift_to_wipe_pos----------------------------------
void steptx_lift_to_wipe_pos(EnFluidCtrlMode mode)
{
	switch (mode)
	{
	case ctrl_wipe:		sok_send_2(&_step_socket[0], CMD_CAP_WIPE_POS,	  0, NULL); break;
	case ctrl_cap:		sok_send_2(&_step_socket[0], CMD_CAP_CAPPING_POS, 0, NULL); break;
	case ctrl_wetwipe:	sok_send_2(&_step_socket[0], CMD_CAP_WETWIPE_POS, 0, NULL); break;
	case ctrl_vacuum:	sok_send_2(&_step_socket[0], CMD_CAP_VACUUM_POS,  0, NULL); break;
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
	case ctrl_wetwipe:	return _Status[0].robinfo.z_in_wetwipe;
	case ctrl_vacuum:	return _Status[0].robinfo.z_in_vacuum; 
	default: return TRUE;
	}
}

//--- steptx_lift_to_print_pos --------------------------------
void steptx_lift_to_print_pos(void)
{
	if (RX_Config.printer.type==printer_test_table) sok_send_2(&_step_socket[0], CMD_CAP_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
	else // TX801/TX802
	{
		INT32 height = RX_Config.stepper.print_height + plc_get_thickness();
		sok_send_2(&_step_socket[0], CMD_CAP_PRINT_POS, sizeof(height), &height);		
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
	sok_send_2(&_step_socket[0], CMD_CAP_UP_POS, 0, NULL);		
}

//--- steptx_lift_in_up_pos --------------
int	 steptx_lift_in_up_pos(void)
{
	return _Status[0].info.z_in_ref;
}

//--- steptx_lift_stop ------------------------------
void steptx_lift_stop(void)
{
	sok_send_2(&_step_socket[0], CMD_CAP_STOP, 0, NULL);
}

//--- steptx_rob_reference_done ---------------------
int steptx_rob_reference_done(void)
{
	return _Status[1].robinfo.ref_done;
}

//--- steptx_rob_do_reference -----------------------
void steptx_rob_do_reference(void)
{
	sok_send_2(&_step_socket[1], CMD_CLN_REFERENCE, 0, NULL);
}

//--- steptx_rob_to_center_pos ----------------------
void steptx_rob_to_center_pos(void)
{
	int pos = 1;
	sok_send_2(&_step_socket[1], CMD_CLN_SHIFT_LEFT, sizeof(pos), &pos);
}

//--- steptx_rob_to_wipe_pos ------------------------
void steptx_rob_to_wipe_pos(EnFluidCtrlMode mode)
{
	if (_step_socket[1]==INVALID_SOCKET) return;
	
	int function=0;
	// 0=CAP
	// 1=Wet Whipe
	// 2=Vacuum
	// 3=Wipe
	// 4=Vacuum change
	switch(mode)
	{
	case ctrl_cap:			function=0; break;
	case ctrl_wetwipe:		function = 1; break;
	case ctrl_vacuum:		function = 2; break;
	case ctrl_wipe:			function = 3; break;
	case ctrl_vacuum_step6: function = 4; break;
	default: break;
	}
	
	sok_send_2(&_step_socket[1], CMD_CLN_MOVE_POS, sizeof(function), &function);		
}
	
//--- steptx_rob_in_wipe_pos ------------------------
int  steptx_rob_in_wipe_pos(EnFluidCtrlMode mode)
{
	if (_step_socket[1]==INVALID_SOCKET) return TRUE;
	
	switch (mode)
	{
	case ctrl_cap:			return _Status[1].robinfo.cap_ready			&& _Status[1].robinfo.moving == FALSE; break;
	case ctrl_wetwipe:		return _Status[1].robinfo.wetwipe_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case ctrl_vacuum:		return _Status[1].robinfo.vacuum_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case ctrl_wipe:			return _Status[1].robinfo.wipe_ready		&& _Status[1].robinfo.moving == FALSE; break;
	case ctrl_vacuum_step6: return _Status[1].robinfo.vacuum_in_change	&& _Status[1].robinfo.moving == FALSE; break;
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
	case ctrl_wetwipe:
	case ctrl_vacuum:	function = 3; break;
	default: break;
	}
	sok_send_2(&_step_socket[1], CMD_CLN_SHIFT_MOV, sizeof(function), &function);		
}

//--- steptx_rob_wipe_done --------------------------------------
int	 steptx_rob_wipe_done(EnFluidCtrlMode mode)
{
	if (_step_socket[1]==INVALID_SOCKET) return TRUE;
	switch (mode)
	{
	case ctrl_wipe:		return _Status[1].robinfo.wipe_done; break;
	case ctrl_wetwipe:	return _Status[1].robinfo.wetwipe_done; break;
	case ctrl_vacuum:	return _Status[1].robinfo.vacuum_done; break;
	default:			return FALSE; break;
	}
	
}

//--- steptx_rob_stop ------------------------------
void steptx_rob_stop(void)
{
	sok_send_2(&_step_socket[1], CMD_CLN_STOP, 0, NULL);
}
