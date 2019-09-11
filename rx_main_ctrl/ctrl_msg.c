// ****************************************************************************
//
//	ctrl_msg.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes -----------------------------------------------------------------
#include <stdio.h>
#include "rx_sok.h"
#include "rx_common.h"
#include "rx_def.h"
#include "rx_threads.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "args.h"
#include "ctr.h"
#include "ctrl_svr.h"
#include "fluid_ctrl.h"
#include "print_ctrl.h"
#include "ctrl_msg.h"
#include "network.h"
#include "step_ctrl.h"

//--- Defines -----------------------------------------------------------------
	

//--- Externals ---------------------------------------------------------------


//--- Modlue Globals -----------------------------------------------------------------

static SFSDirEntry _RfsInfo[HEAD_BOARD_CNT];

//--- Prototypes ------------------------------------------------------
static int _do_head_stat		(RX_SOCKET socket, int headNo, SHeadBoardStat	*pstat);
static int _do_trace_evt		(RX_SOCKET socket, STraceMsg		*msg);
static void _do_fluidCtrlMode	(RX_SOCKET socket, SFluidCtrlCmd	*pmsg); 
static int _do_save_file_hdr	(RX_SOCKET socket, int headNo, SFSDirEntry		*msg); 
static int _do_save_file_blk	(RX_SOCKET socket, int headNo, SDataBlockMsg	*msg); 

//--- Statics -----------------------------------------------------------------

//--- handle_headCtrl_msg ------------------------------------------
int handle_headCtrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;
	int headNo = *((int*)par);
	static int _err=FALSE;
	
	//--- handle the message --------------
//	ctrl_head_alive(headNo);
	net_register_by_device(dev_head, headNo);
	
	reply = REPLY_OK;
	SMsgHdr *phdr = (SMsgHdr*)msg;

	TrPrintfL(TRUE, "received Head[%d].MsgId=0x%08x", headNo, phdr->msgId);
	
	switch (phdr->msgId)
	{
	case REP_PING:					TrPrintf			(TRUE, "got REP_PING");							break;
	case REP_HEAD_BOARD_CFG:		ctrl_head_cfg_done	(headNo);										break;
	case REP_HEAD_STAT:				_do_head_stat		(socket, headNo, (SHeadBoardStat*)&phdr[1]);	break;
	case CMD_FLUID_CTRL_MODE:		_do_fluidCtrlMode	(socket, (SFluidCtrlCmd*) msg);					break;
	case EVT_TRACE:					_do_trace_evt		(socket, (STraceMsg*)	  msg);					break;
	case EVT_GET_EVT:				ctrl_do_log_evt		(socket, (SLogMsg*)		  msg);					break;
	case EVT_PRINT_DONE:			pc_print_done		(headNo, (SPrintDoneMsg*) msg);					break;
	case CMD_RFS_SAVE_FILE_HDR:		_do_save_file_hdr	(socket, headNo, (SFSDirEntry*)   msg);			break;
	case CMD_RFS_SAVE_FILE_BLOCK:	_do_save_file_blk	(socket, headNo, (SDataBlockMsg*) msg);			break;
	default:		if (!_err)
					{
						EDevice device;
						int no;
						char addr[32];
						_err=TRUE;
						ctrl_get_device(socket, &device, &no);
						sok_get_addr_str(sender, addr, sizeof(addr));
						ErrorEx(device, no, ERR_CONT, 0, "Unknown Command 0x%04x, sent by %s[%d] >>%s<<", phdr->msgId, DeviceStr[device], no, addr);
						reply = REPLY_ERROR;
					}
	}

	return reply;
}

//--- _do_head_stat ----------------------------------------------------
static int _do_head_stat(RX_SOCKET socket, int headNo, SHeadBoardStat	*pstat)
{
	int i;
	
	if (headNo<HEAD_BOARD_CNT)
	{
		ctrl_head_alive(headNo);
		ctr_set_total(pstat->machineMeters);
		memcpy(&RX_HBStatus[headNo], pstat, sizeof(RX_HBStatus[0]));
		if (RX_HBStatus[headNo].err & 
			( err_fpga_overheated
		//	| err_head_pcb_overheated
			| err_amc7891
			| err_pwr_all_off
			| err_amp_all_on
			| err_pwr_all_on))
		{
			fluid_send_ctrlMode(-1, ctrl_off, TRUE);
		}			
		
		for (i=0; i<SIZEOF(pstat->head); i++)
		{
			int inksupply = RX_Config.headBoard[headNo].head[i].inkSupply;
			if (RX_Config.headBoard[headNo].head[i].enabled)
			{
				if (pstat->head[i].ctrlMode==ctrl_undef)
				{
					if (fluid_get_ctrlMode(inksupply)!=ctrl_undef)
						ctrl_send_head_fluidCtrlMode(headNo*MAX_HEADS_BOARD+i, fluid_get_ctrlMode(inksupply), FALSE, FALSE);
				}
				fluid_set_head_state(inksupply, &pstat->head[i]);					
			}
		}
	}
	return REPLY_OK;
}

//--- _do_fluidCtrlMode ---
static void _do_fluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd *pmsg)
{
    fluid_send_ctrlMode(pmsg->no, pmsg->ctrlMode, TRUE);
}

//--- _do_trace_evt -------------------------------------------
static int _do_trace_evt(RX_SOCKET socket, STraceMsg *msg)
{
	EDevice device;
	int no;
	
	ctrl_get_device(socket, &device, &no);
	TrPrintf(TRUE, "%s %d-Trace: %s", DeviceStr[device], no + 1, msg->message);
	return REPLY_OK;
}

//--- ctrl_do_log_evt -----------------------------------------------------
int ctrl_do_log_evt(RX_SOCKET socket, SLogMsg *msg)
{
	EDevice device;
	int no;
	
	ctrl_get_device(socket, &device, &no);
	SlaveError(device, no, &msg->log);
	if (msg->log.logType==LOG_TYPE_ERROR_ABORT && !arg_simuPLC) pc_abort_printing();
	return REPLY_OK;
}

//--- _do_save_file_hdr ---------------------
static int _do_save_file_hdr	(RX_SOCKET socket, int headNo, SFSDirEntry	*msg)
{	
	SFSDirEntry *hdr= &_RfsInfo[headNo];
	char		path[MAX_PATH];
	char		fname[MAX_PATH];
	char		ext[10];
	FILE		*f;
	int i;
		
	memcpy(&_RfsInfo[headNo], msg, sizeof(_RfsInfo[headNo]));
	split_path(hdr->name, path, fname, ext);	
	sprintf(path, PATH_FPGA_REGS "head_%d", headNo+1);
	rx_mkdir(path);
	for (i=0; ; i++)
	{
		sprintf(path, PATH_FPGA_REGS "head_%d/%s_%d%s", headNo+1, fname, i, ext);
		if (!rx_file_exists(path))
		{
			f=fopen(path, "wb");
			hdr->timeModified = (UINT64)f;
			break;
		}
	}
	
	return REPLY_OK;
}

//--- _do_save_file_blk ----------------------
static int _do_save_file_blk	(RX_SOCKET socket, int headNo, SDataBlockMsg *msg)
{
	SFSDirEntry *hdr= &_RfsInfo[headNo];
	FILE		*f	= (FILE*)hdr->timeModified;
	int			len;
	
	if (f!=NULL)
	{
		len=fwrite(msg->data, 1, msg->hdr.msgLen-sizeof(msg->hdr), f);
		if (len >= (int)hdr->size)
		{
			fclose(f);
			memset(hdr, 0, sizeof(hdr));
		}
		else hdr->size -= len;		
	}
		
	return REPLY_OK;
}
		