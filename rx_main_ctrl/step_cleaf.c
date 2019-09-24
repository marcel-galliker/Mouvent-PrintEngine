// ****************************************************************************
//
//	step_cleaf.c		Stepper Control for CLEAF project
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
#include "args.h"
#include "cleaf_orders.h"
#include "step_cleaf.h"
#include "step_ctrl.h"

//--- defines -----------------------------------------------------------------------------------------------------------------------------

#define trace TRUE // FALSE

#define STEPPER_CNT		4



//--- global  variables -----------------------------------------------------------------------------------------------------------------

static RX_SOCKET		_step_socket[STEPPER_CNT];
static SStepperStat		_status[STEPPER_CNT];
static int				_splicing=FALSE;
static int				_enc_disabled=FALSE;
static int				_allow_stepper_move_down = FALSE;	// if drip pans below the heads, disable the move down of the steppers motors

//--- prorotypes -------------------------------------------------


//--- stepc_init ---------------------------------------------------
void stepc_init(int no, RX_SOCKET psocket)
{
	if (no >= 0 && no < STEPPER_CNT)
	{
		_step_socket[no] = psocket;
	}
	memset(_status, 0, sizeof(_status));
}

//--- stepc_error_reset ----------------------------------------------------
void stepc_error_reset(void)
{
}

//--- stepc_handle_gui_msg------------------------------------------------------------------
int	 stepc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{
	for (int no = 0; no < SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] != INVALID_SOCKET)
		{
			switch (cmd)
			{
			case CMD_CAP_STOP:
				sok_send_2(&_step_socket[no], CMD_CAP_STOP, 0, NULL);
				break;

			case CMD_CAP_REFERENCE:
				sok_send_2(&_step_socket[no], CMD_CAP_REFERENCE, 0, NULL); // ref Lift
				break;

			case CMD_CAP_UP_POS:
				if (!arg_simuPLC && RX_StepperStatus.info.DripPans_InfeedDOWN && RX_StepperStatus.info.DripPans_OutfeedDOWN) sok_send_2(&_step_socket[no], CMD_CAP_UP_POS, 0, NULL);
				break;

			case CMD_CAP_PRINT_POS:
				if ( RX_StepperStatus.info.DripPans_InfeedDOWN && RX_StepperStatus.info.DripPans_OutfeedDOWN)sok_send_2(&_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);			
				break;

			case CMD_CAP_CAPPING_POS:
				sok_send_2(&_step_socket[no], CMD_CAP_CAPPING_POS, 0, NULL);
				break;
				
			case CMD_CLN_DRIP_PANS:
				sok_send_2(&_step_socket[no], CMD_CLN_DRIP_PANS, 0, NULL);
				break;
			}
		}
	}
	return REPLY_OK;
}

//--- stepc_handle_status ----------------------------------------------------------------------
int stepc_handle_status(int no, SStepperStat *pStatus)
{
	int cnt=0;
	int change=FALSE;
	int allowed = FALSE;
	ETestTableInfo info;
	
	if (_status[no].info.splicing	  != pStatus->info.splicing)	 {TrPrintfL(TRUE, "Stepper[%d]: new info.splice=%d",	   no, pStatus->info.splicing); change=TRUE;}
	if (_status[no].info.printhead_en != pStatus->info.printhead_en) {TrPrintfL(TRUE, "Stepper[%d]: new info.printhead_en=%d", no, pStatus->info.printhead_en); change=TRUE;}
	
	memcpy(&_status[no], pStatus, sizeof(_status[no]));

	memset(&info, 0, sizeof(info));
	info.ref_done		= TRUE;
	info.z_in_ref		= TRUE;
	info.headUpInput_0	= TRUE;
	info.headUpInput_1	= TRUE;
	info.headUpInput_2	= TRUE;
	info.headUpInput_3	= TRUE;
	info.DripPans_InfeedDOWN	= TRUE;
	info.DripPans_InfeedUP		= TRUE;
	info.DripPans_OutfeedDOWN	= TRUE;
	info.DripPans_OutfeedUP		= TRUE;
	info.z_in_print		= TRUE;
	info.z_in_cap		= TRUE;
	info.x_in_ref		= TRUE;
	info.printhead_en	= TRUE;
	//	TrPrintf(TRUE, "steptest_handle_status(%d)", no);

	RX_StepperStatus.posX = 0;
	RX_StepperStatus.posY = 0;
	RX_StepperStatus.posZ = 0;
	
	info.DripPans_InfeedDOWN	= _status[0].info.DripPans_InfeedDOWN;
	info.DripPans_InfeedUP		= _status[0].info.DripPans_InfeedUP;
	info.DripPans_OutfeedDOWN	= _status[0].info.DripPans_OutfeedDOWN;
	info.DripPans_OutfeedUP		= _status[0].info.DripPans_OutfeedUP;
	allowed = info.DripPans_InfeedDOWN && info.DripPans_OutfeedDOWN && !info.DripPans_InfeedUP && !info.DripPans_OutfeedUP;

	for (int i = 0; i < STEPPER_CNT; i++)
	{
		if (_step_socket[i] != INVALID_SOCKET)
		{
			//			TrPrintf(TRUE, "Stepper[%d]: ref_done=%d moving=%d  in_print=%d  up=%d", i, _status[i].info.ref_done, _status[i].info.moving, _status[i].info.z_in_print, _status[i].info.z_in_ref);
			cnt++;
			info.ref_done		&= _status[i].info.ref_done;
			info.moving			|= _status[i].info.moving;
			info.z_in_ref		&= _status[i].info.z_in_ref;
			info.z_in_print		&= _status[i].info.z_in_print;
			info.z_in_cap		&= _status[i].info.z_in_cap;
			info.x_in_ref		&= _status[i].info.x_in_ref;
			info.printhead_en	&= _status[i].info.printhead_en;
			info.splicing		|= _status[i].info.splicing;
			info.x_in_cap		= TRUE;

			RX_StepperStatus.posX += _status[i].posX;
			RX_StepperStatus.posY += _status[i].posY;
			RX_StepperStatus.posZ += _status[i].posZ;
			
			if (allowed != _allow_stepper_move_down)
			{
				if (allowed) sok_send_2(&_step_socket[i], CMD_CAP_ALLOW_MOVE_DOWN, 0, NULL);
				else sok_send_2(&_step_socket[i], CMD_CAP_NOT_ALLOW_MOVE_DOWN, 0, NULL);
			}
		}
	}
	//--- right->left on GUI!
	info.headUpInput_3 = _status[0].info.headUpInput_0 && _status[0].info.headUpInput_1;
	info.headUpInput_2 = _status[1].info.headUpInput_0 && _status[1].info.headUpInput_1;
	info.headUpInput_1 = _status[2].info.headUpInput_0 && _status[2].info.headUpInput_1;
	info.headUpInput_0 = _status[3].info.headUpInput_0 && _status[3].info.headUpInput_1;
	
	_allow_stepper_move_down = allowed;

	if (cnt)
	{
		RX_StepperStatus.posX /= cnt;
		RX_StepperStatus.posY /= cnt;
		RX_StepperStatus.posZ /= cnt;		
	}
	
	if (change) TrPrintfL(TRUE, "Splice changed: _splicing=%d, info.splicing=%d", _splicing, info.splicing);
	
	if (!_splicing && info.splicing)
	{
		enc_enable_printing(FALSE);
		_enc_disabled = TRUE;
	}
	if (info.z_in_print && _enc_disabled)
	{
		enc_enable_printing(TRUE);
		_enc_disabled=FALSE;
		if (plc_is_splicing()) co_roll_end();
	}

	if (_splicing && !info.splicing) 
	{
		TrPrintfL(TRUE, "Splice end");
		if (RX_PrinterStatus.printState==ps_printing) 
		{
			TrPrintfL(TRUE, "stepc_to_print_pos ");
			stepc_to_print_pos();
		}
	}
	_splicing = info.splicing;

	//	TrPrintf(TRUE, "STEPPER: ref_done=%d moving=%d  in_print=%d  up=%d", info.ref_done, info.moving, info.z_in_print, info.z_in_ref);

	if (info.z_in_print != RX_StepperStatus.info.z_in_print) Error(LOG, 0, "z_in_print changed to %d", info.z_in_print);
	
	memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));

	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);

	return REPLY_OK;
}

//--- stepc_abort_printing --------------------
int stepc_abort_printing(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
	return REPLY_OK;
}

//--- stepc_to_print_pos --------------------------------
int	 stepc_to_print_pos(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, NULL, 0);
	return REPLY_OK;
}

//--- stepc_to_up_pos --------------------------------
int	 stepc_to_up_pos(void)
{
	stepc_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
	return REPLY_OK;
}
