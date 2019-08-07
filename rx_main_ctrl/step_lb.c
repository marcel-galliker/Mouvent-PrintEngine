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
#include "step_lb.h"

#define STEPPER_CNT		4

static RX_SOCKET		*_step_socket[STEPPER_CNT]={0};

static SStepperStat		_Status[STEPPER_CNT];
static int				_AbortPrinting=FALSE;

//--- steplb_init ---------------------------------------------------
void steplb_init(int no, RX_SOCKET *psocket)
{
	if (no>=0 && no<STEPPER_CNT)
	{
		_step_socket[no] = psocket;
	}
	memset(_Status, 0, sizeof(_Status));
}

//--- steplb_handle_gui_msg------------------------------------------------------------------
int	 steplb_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{	
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && *_step_socket[no]!=INVALID_SOCKET)
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
						sok_send_2(_step_socket[no], cmd, 0, NULL);
						break;

			case CMD_TT_SCAN:
						{
							STestTableScanPar par;
							par.speed	= 5;
							par.scanCnt = 5;
							par.scanMode= PQ_SCAN_STD;
							par.yStep   = 10000;

							sok_send_2(_step_socket[no], CMD_TT_SCAN, sizeof(par), &par);
						}
						break;

			//--- cappping ---------------------------------------------------------
			case CMD_CAP_STOP:
			case CMD_CAP_UP_POS:
			case CMD_CAP_CAPPING_POS:
						sok_send_2(_step_socket[no], cmd, 0, NULL);
						break;

			case CMD_CAP_REFERENCE:
						TrPrintfL(TRUE, "Stepper[%d].CMD_CAP_REFERENCE", no);
						sok_send_2(_step_socket[no], cmd, 0, NULL);
						break;
		
			case CMD_CAP_PRINT_POS:
						_AbortPrinting=FALSE;
						sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
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
	ETestTableInfo info;
	memcpy(&_Status[no], pStatus, sizeof(_Status[no]));

	memset(&info, 0, sizeof(info));
	info.printhead_en	= TRUE;
	info.ref_done		= TRUE;
	info.z_in_ref		= TRUE;
	info.z_in_print		= TRUE;
	info.z_in_cap		= TRUE;
	info.x_in_cap		= TRUE;
			
//	TrPrintf(TRUE, "steplb_handle_Status(%d)", no);
	
	for (i=0; i<STEPPER_CNT; i++)
	{
		if (_step_socket[i] && *_step_socket[i]!=INVALID_SOCKET)
		{
//			TrPrintf(TRUE, "Stepper[%d]: ref_done=%d moving=%d  z_in_print=%d  z_in_ref=%d", i, _Status[i].info.ref_done, _Status[i].info.moving, _Status[i].info.z_in_print, _Status[i].info.z_in_ref);
			info.ref_done		&= _Status[i].info.ref_done;
			info.printhead_en	&= _Status[i].info.printhead_en;
			info.moving			|= _Status[i].info.moving;
			info.z_in_ref		&= _Status[i].info.z_in_ref;
			info.z_in_print		&= _Status[i].info.z_in_print;
			info.z_in_cap		&= _Status[i].info.z_in_cap;
			info.x_in_cap		&= _Status[i].info.x_in_cap;
			if (_Status[i].info.moving) 
			{
				RX_StepperStatus.posX = _Status[i].posX;
				RX_StepperStatus.posY = _Status[i].posY;
				RX_StepperStatus.posZ = _Status[i].posZ;
			}
		}
	};
	
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
	
	if (_AbortPrinting && RX_StepperStatus.info.z_in_print) steplb_to_up_pos();
		
	memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));
	RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();

	gui_send_msg_2(0, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return REPLY_OK;
}

//--- steplb_to_print_pos --------------------------------
int	 steplb_to_print_pos(void)
{
	int no;
	_AbortPrinting = FALSE;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && *_step_socket[no]!=INVALID_SOCKET)
		{
			sok_send_2(_step_socket[no], CMD_CAP_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
		}
	}
	return REPLY_OK;									
}

//--- steplb_abort_printing -----------------------------------------
int  steplb_abort_printing(void)
{
	
	if(RX_StepperStatus.info.z_in_print) return steplb_to_up_pos();
	else _AbortPrinting = TRUE;
	return REPLY_OK;
}

//--- steplb_to_up_pos ---------------------------
int	 steplb_to_up_pos(void)
{
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && *_step_socket[no]!=INVALID_SOCKET)
		{
			sok_send_2(_step_socket[no], CMD_CAP_UP_POS, 0, NULL);
		}
	}
	_AbortPrinting = FALSE;
	return REPLY_OK;									
}

//--- steplb_is_printing ----------------------------------------------
int	 steplb_is_printing(int isprinting)
{
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no] && *_step_socket[no]!=INVALID_SOCKET)
		{
			sok_send_2(_step_socket[no], CMD_CAP_IS_PRINTING, sizeof(isprinting), &isprinting);
		}
	}
	return REPLY_OK;													
}

