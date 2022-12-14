// ****************************************************************************
//
//	step_std.c	Standrd Stepper Control		
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

static RX_SOCKET _step_socket=INVALID_VALUE;
static int		  _Curing;

//--- steps_init ---------------------------------------------------
void steps_init(RX_SOCKET psocket)
{
	_step_socket = psocket;
}

//--- steps_handle_gui_msg------------------------------------------------------------------
int	 steps_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
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
				sok_send_2(&_step_socket, cmd, 0, NULL);
				break;

	case CMD_TT_SCAN:
				{
					STestTableScanPar par;
					par.speed	= 5;
					par.scanCnt = 5;
					par.scanMode= PQ_SCAN_STD;
					par.yStep   = 10000;

					sok_send_2(&_step_socket, CMD_TT_SCAN, sizeof(par), &par);
				}
				break;

	//--- cappping ---------------------------------------------------------
	case CMD_LIFT_STOP:
	case CMD_LIFT_REFERENCE:
	case CMD_LIFT_UP_POS:
	case CMD_LIFT_CAPPING_POS:
				sok_send_2(&_step_socket, cmd, 0, NULL);
				break;
		
	case CMD_LIFT_PRINT_POS:
				step_lift_to_print_pos();	
				break;
	}
	return REPLY_OK;
}

//--- steps_set_curing ---------------------------------------
void steps_set_curing(int curing)
{
	_Curing = curing;
}

//--- steps_handle_status ----------------------------------------------------------------------
int steps_handle_status(SStepperStat *pStatus)
{
	memcpy(&RX_StepperStatus, pStatus, sizeof(RX_StepperStatus));
	RX_StepperStatus.info.uv_on    = enc_is_uv_on();
	RX_StepperStatus.info.uv_ready = enc_is_uv_ready();
	RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();
		
	if (_Curing && !RX_StepperStatus.info.printing && !RX_StepperStatus.info.curing)
	{
		_Curing = FALSE;
		pc_abort_printing();									
	}
	gui_send_msg_2(INVALID_SOCKET, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return REPLY_OK;
}

//--- steps_to_print_pos --------------------------------
int	 steps_to_print_pos(void)
{
	if (RX_Config.printer.type==printer_test_table) sok_send_2(&_step_socket, CMD_LIFT_PRINT_POS, sizeof(UINT32), &RX_Config.stepper.cap_height);
	else // TX801/TX802
	{
		INT32 height = RX_Config.stepper.print_height + RX_Config.stepper.material_thickness;
		sok_send_2(&_step_socket, CMD_LIFT_PRINT_POS, sizeof(height), &height);		
	}
	return REPLY_OK;									
}
