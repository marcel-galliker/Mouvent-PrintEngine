// ****************************************************************************
//
//	step_test.c	Standrd Stepper Control		
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

#define STEPPER_CNT		4

static RX_SOCKET	_step_socket[STEPPER_CNT]={0};
static SStepperStat	_status[STEPPER_CNT];

//--- steptest_init ---------------------------------------------------
void steptest_init(int no, RX_SOCKET psocket)
{
	if (no>=0 && no<STEPPER_CNT)
	{
		_step_socket[no] = psocket;
	}
	memset(_status, 0, sizeof(_status));
}

//--- steptest_handle_gui_msg------------------------------------------------------------------
int	 steptest_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{	
	int no;
	for (no=0; no<SIZEOF(_step_socket); no++)
	{
		if (_step_socket[no]!=INVALID_SOCKET)
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
			case CMD_LIFT_STOP:
			case CMD_LIFT_REFERENCE:
			case CMD_LIFT_UP_POS:
			case CMD_LIFT_CAPPING_POS:
						sok_send_2(&_step_socket[no], cmd, 0, NULL);
						break;
		
			case CMD_LIFT_PRINT_POS:
						sok_send_2(&_step_socket[no], CMD_LIFT_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
						break;
			}
		}
	}
	return REPLY_OK;
}

//--- steptest_handle_status ----------------------------------------------------------------------
int steptest_handle_status(int no, SStepperStat *pStatus)
{
	int i;
	ETestTableInfo info;
	memcpy(&_status[no], pStatus, sizeof(_status[no]));

	memset(&info, 0, sizeof(info));
	info.ref_done	= TRUE;
	info.z_in_ref	= TRUE;
	info.z_in_print = TRUE;
	info.z_in_cap	= TRUE;
	info.x_in_cap	= TRUE;
	
//	TrPrintf(TRUE, "steptest_handle_status(%d)", no);
	
	for (i=0; i<STEPPER_CNT; i++)
	{
		if (_step_socket[i]!=INVALID_SOCKET)
		{
//			TrPrintf(TRUE, "Stepper[%d]: ref_done=%d moving=%d  in_print=%d  up=%d", i, _status[i].info.ref_done, _status[i].info.moving, _status[i].info.z_in_print, _status[i].info.z_in_ref);
			info.ref_done		&= _status[i].info.ref_done;
			info.moving			|= _status[i].info.moving;
			info.curing			|= _status[i].info.curing;
			info.z_in_ref		&= _status[i].info.z_in_ref;
			info.z_in_print		&= _status[i].info.z_in_print;
			info.z_in_cap		&= _status[i].info.z_in_cap;
			info.x_in_cap		&= _status[i].info.x_in_cap;
            RX_StepperStatus.posY[i] = _status[i].posY[0];
			if (_status[i].info.moving) 
			{
				RX_StepperStatus.posX = _status[i].posX;
				RX_StepperStatus.posZ = _status[i].posZ;
                RX_StepperStatus.posZ_back = _status[i].posZ_back;
			}
		}
	};
	
//	TrPrintf(TRUE, "STEPPER: ref_done=%d moving=%d  in_print=%d  up=%d", info.ref_done, info.moving, info.z_in_print, info.z_in_ref);
	
	if (!info.moving) 
	{
		RX_StepperStatus.posX = _status[no].posX;
		RX_StepperStatus.posZ = _status[no].posZ;
        RX_StepperStatus.posZ_back = _status[no].posZ_back;
	}
	
	if (RX_StepperStatus.info.curing && !info.curing) pc_abort_printing();

	memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));
	RX_StepperStatus.info.x_in_cap = plc_in_cap_pos();

	gui_send_msg_2(INVALID_SOCKET, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return REPLY_OK;
}

//--- steptest_to_print_pos --------------------------------
int	 steptest_to_print_pos(void)
{
//	Error(LOG, 0, "Setting Printhead Height to %d", RX_Config.stepper.print_height);
	sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
	return REPLY_OK;									
}
