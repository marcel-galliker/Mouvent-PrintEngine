// *************	***************************************************************
//
//	setup_assist.h		
//
// ****************************************************************************
//
//	Copyright 2020 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "gui_svr.h"
#include "tcp_ip.h"
#include "sa_tcp_ip.h"
#include "plc_ctrl.h"
#include "setup_assist.h"


//--- statics ---------------------------------------------
static int			_SaThreadRunning = TRUE;
static RX_SOCKET	_SaSocket = INVALID_SOCKET;
static const double	_microns2steps = 30.000;

//--- prorotypes -----------------------------------------
static void *_sa_thread(void *lpParameter);
static void *_sa_tick_thread(void *lpParameter);
static int _sa_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _do_sa_stat(RX_SOCKET socket, SSetupAssist_StatusMsg	*pstat);

//--- sa_init -----------------------------------------
int	 sa_init(void)
{
	_SaThreadRunning = TRUE;
	rx_thread_start(_sa_thread, NULL, 0, "_sa_thread");		
	rx_thread_start(_sa_tick_thread, NULL, 0, "_sa_tick_thread");		
	return REPLY_OK;
}

//--- sa_end -------------------------------------------
int  sa_end(void)
{
	_SaThreadRunning = FALSE;
	return REPLY_OK;
}

//--- sa_connected -------------------------------------
int	sa_connected(void)
{
	return (_SaSocket!=INVALID_SOCKET);
}

//--- _sa_thread ----------------------------------------
static void *_sa_thread(void *lpParameter)
{
	int ret;
	int err=FALSE;
	
	while (_SaThreadRunning)
	{
		if (_SaSocket==INVALID_SOCKET)
		{
			if (sok_open_client(&_SaSocket, SETUP_ASSIST_IP_ADDR, SETUP_ASSIST_PORT, SOCK_STREAM)==REPLY_OK)
			{
				sok_receiver(NULL, &_SaSocket, _sa_handle_msg, NULL);
			}
		}
		
		rx_sleep(1000);
	}
	return NULL;
}

//--- _sa_tick_thread ---------------------------------------------
static void* _sa_tick_thread(void *lpParameter)
{
	while (_SaThreadRunning)
	{
		int ret=sok_send_2(&_SaSocket, CMD_STATUS_GET, 0, NULL);
		TrPrintfL(TRUE,"SetupAssist(Socket=%d): CMD_STATUS_GET, ret=%d", _SaSocket, ret);
		rx_sleep(1000);
	}
	return NULL;
}


//--- _sa_handle_msg -------------------------------------
static int _sa_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{	
	//--- handle the message --------------
	SMsgHdr *phdr = (SMsgHdr*)msg;
	switch(phdr->msgId)
	{
		case CMD_STATUS_GET: _do_sa_stat(socket, (SSetupAssist_StatusMsg*) msg); break;
		default:
		Error(ERR_CONT, 0, "Unknown Command 0x%04x, sent by Setup-Assist >>%s<<", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _do_sa_stat ---------------------------------------------------------------
static int _do_sa_stat(RX_SOCKET socket, SSetupAssist_StatusMsg	*pstat)
{
	TrPrintfL(TRUE, "SetupAssist: Got Status");
	pstat->header.msgId = REP_SETUP_ASSIST_STAT;
	pstat->motorPosition *= _microns2steps;
	gui_send_msg(INVALID_SOCKET, pstat);
	return REPLY_OK;
}

//--- sa_handle_gui_msg -------------------------------------------------------
void sa_handle_gui_msg(RX_SOCKET socket, void *pmsg_)
{
	SetupAssist_MoveCmd *pmsg = (SetupAssist_MoveCmd*)pmsg_;
	switch (pmsg->header.msgId)
	{
    case CMD_SA_REFERENCE:		Error(LOG, 0, "Send CMD_MOTOR_REFERENCE");
								sok_send_2(&_SaSocket, CMD_MOTOR_REFERENCE,    0, NULL); 
								break;

    case CMD_SA_STOP:			Error(LOG, 0, "Send CMD_SA_STOP");
								sok_send_2(&_SaSocket, CMD_SA_STOP,    0, NULL); 
								break;

    case CMD_SA_MOVE:			Error(LOG, 0, "Send CMD_MOTOR_MOVE");
								{
									SetupAssist_MoveCmd cmd;
									cmd.header.msgLen = sizeof(cmd);
									cmd.header.msgId  = CMD_MOTOR_MOVE;
									cmd.steps		  = (INT32)((double)pmsg->steps/_microns2steps);
									cmd.speed		  = 200;
									cmd.acc			  = 100;
									cmd.current		  = 300;
									sok_send(&_SaSocket, &cmd);
								}
								break;

    case CMD_SA_OUT_TRIGGER:	Error(LOG, 0, "Send CMD_SET_DENSIO_TRIGGER");
								{
									SetupAssist_OutTriggerCmd cmd;
									cmd.header.msgLen = sizeof(cmd);
									cmd.header.msgId  = CMD_OUT_TRIGGER;
									cmd.out			  = 0;
									cmd.time_ms		  = 100;
									sok_send(&_SaSocket, &cmd);
								}
								break;

    case CMD_SA_WEB_MOVE:		Error(LOG, 0, "Send CMD_SA_WEB_MOVE");
								SetupAssist_MoveCmd *pcmd = (SetupAssist_MoveCmd*)pmsg;
								plc_move_web(pcmd->steps);
								break;

    case CMD_SA_WEB_STOP:		Error(LOG, 0, "Send CMD_SA_WEB_STOP");
								plc_pause_printing(FALSE);
								break;

    default: Error(WARN, 0, "Unknown Command 0x%08x", pmsg->header.msgId);
	}
}