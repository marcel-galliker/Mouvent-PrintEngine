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

static RX_SOCKET	_step_socket[STEPPER_CNT]={0};
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

//--- steptx_to_print_pos --------------------------------
int	 steptx_to_print_pos(void)
{
	if (RX_Config.printer.type==printer_test_table) sok_send_2(&_step_socket[0], CMD_CAP_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
	else // TX801/TX802
	{
		INT32 height = RX_Config.stepper.print_height + plc_get_thickness();
		sok_send_2(&_step_socket[0], CMD_CAP_PRINT_POS, sizeof(height), &height);		
	}
	return REPLY_OK;									
}

//--- steptx_to_purge_pos ----------------------------------
int	 steptx_to_purge_pos(int no)
{
	Error(WARN, 0, "steptx_to_purge_pos NOT Implemented");
	return REPLY_OK;
}

//--- steptx_in_purge_pos -----------------------------------
int  steptx_in_purge_pos(void)
{
	Error(WARN, 0, "steptx_in_purge_pos NOT Implemented");
	return TRUE;			
}

//--- steptx_lift_is_up --------------
int	 steptx_lift_is_up(void)
{
	return RX_StepperStatus.info.z_in_ref;
}

//--- steptx_wipe_start -------------------------------------
int	 steptx_wipe_start(int no)
{
	Error(WARN, 0, "steptx_wipe_start NOT Implemented");
	return REPLY_OK;			
}

//--- steptx_wipe_done --------------------------------------
int	 steptx_wipe_done(void)
{
	Error(WARN, 0, "steptx_wipe_done NOT Implemented");
	return REPLY_OK;			
}
