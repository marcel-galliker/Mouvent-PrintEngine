// ****************************************************************************
//
//	siemens_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "tcp_ip.h"
#include "plc_ctrl.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "setup.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"
#include "network.h"
#include "label.h"
#include "spool_svr.h"
#include "siemens_ctrl.h"

//--- Defines -----------------------------------------------------------------
	
// Siemsns:		192.168.20.192:2000
// Server:		192.168.20.118

//--- Externals ---------------------------------------------------------------

//--- Statics -----------------------------------------------------------------
static RX_SOCKET _Socket=INVALID_SOCKET;

static int		 _SiemensThreadRunning=FALSE;

static int		_IncPerMeter;
static int		_WakeupLen;
static int		_PrintGo_Dist;
static int		_PrintGo_Mode;
static int		_Scanning=FALSE;
static int		_DistTelCnt=0;
static int		_TotalPgCnt;
static int		_StopPG;
static int		_Printing=FALSE;
static int		_Khz=0;
static UINT32	_WarnMarkReaderPos;

//--- Prototypes --------------------------------------------------------------

static void *_siemens_thread(void *lpParameter);
static int   _siemens_closed(RX_SOCKET socket, const char *peerName);

static int  _handle_siemens_msg		(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);

//--- siemens_init -------------------------------------------------
void siemens_init(void)
{
	_Socket=INVALID_SOCKET;
	if(!_SiemensThreadRunning)
	{
		_SiemensThreadRunning = TRUE;
		rx_thread_start(_siemens_thread, NULL, 0, "_siemens_thread");
	}
}

//--- siemensc_end -------------------------------------
void siemens_end(void)
{
	_SiemensThreadRunning = FALSE;		
}

//--- siemens_error_reset -----------------------
void siemens_error_reset(void)
{
	sok_send_2(&_Socket, CMD_ERROR_RESET, 0, NULL);
}

//--- _siemens_thread ------------------------------------------------------------
static void *_siemens_thread(void *lpParameter)
{
	int errNo;
	
	while (_SiemensThreadRunning)
	{
		if (_Socket==INVALID_SOCKET)
		{			
			errNo=sok_open_client_2(&_Socket, "192.168.20.192", 2000, SOCK_STREAM, _handle_siemens_msg, _siemens_closed);
			if (errNo)
			{
			//	char str[256];
			//	Error(ERR_CONT, 0, "Socket Error >>%s<<", err_system_error(errNo, str,  sizeof(str)));
			}
			else
			{
				TrPrintfL(TRUE, "Connected");
				ErrorEx(dev_plc, -1, LOG, 0, "Connected");
			}
		}				
		rx_sleep(1000);
	}
	return NULL;
}

//--- _siemens_closed --------------------------------------------
static int _siemens_closed(RX_SOCKET socket, const char *peerName)
{
	sok_close(&_Socket);
//	TrPrintfL(TRUE, "TCP/IP connection closed");
	Error(LOG, 0, "TCP/IP connection closed");
	return REPLY_OK;
}

//--- siemens_tick --------------------------------------------
void  siemens_tick(void)
{
	if (sok_send_2(&_Socket, CMD_PLC_STAT, 0, NULL)==REPLY_OK) 
	{
	//	TrPrintfL(TRUE, "Siemend: Sent Message");
	}
}

//--- _handle_siemens_msg ------------------------------------------------------------------
static int _handle_siemens_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr	*phdr = (SMsgHdr*)msg;
	if (len)
	{
		net_register_by_device(dev_plc, -1);

//		TrPrintfL(TRUE, "received Siemens.MsgId=0x%08x", phdr->msgId);
		Error(LOG, 0, "received msg (len=%d, id=5x%08x)", phdr->msgLen, phdr->msgId);

		switch(phdr->msgId)
		{
		case REP_ENCODER_STAT:  	Error(LOG, 0, "received REP_ENCODER_STAT"); break;
		case EVT_GET_EVT:			Error(LOG, 0, "received EVT_GET_EVT");		break;
		default: Error(WARN, 0, "Siemens:Got unknown MessageId=0x%08x", phdr->msgId);
		}		
	}
	return REPLY_OK;
}
