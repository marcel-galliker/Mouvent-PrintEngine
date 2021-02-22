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

#define MOVE_TIEMOUT	10
#define TRACE_DEVICE	FALSE

//--- statics ---------------------------------------------
static int			_SaThreadRunning = TRUE;
static RX_SOCKET	_SaSocket = INVALID_SOCKET;
static int			_Timeout=0;
static const double	_microns2steps = 30.000;

static SSetupAssist_StatusMsg	_Status;

//--- prorotypes -----------------------------------------
static void *_sa_thread(void *lpParameter);
static void *_sa_tick_thread(void *lpParameter);
static int _sa_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _do_sa_stat(RX_SOCKET socket, SSetupAssist_StatusMsg	*pstat);
static int _do_sa_trace(RX_SOCKET socket, SSetupAssist_TraceMsg	*ptrace);

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
	int err=FALSE;
	
	while (_SaThreadRunning)
	{
		if (_SaSocket==INVALID_SOCKET)
		{
			if (sok_open_client(&_SaSocket, SETUP_ASSIST_IP_ADDR, SETUP_ASSIST_PORT, SOCK_STREAM)==REPLY_OK)
			{
				UINT32	trace=TRACE_DEVICE;
				int ret=sok_send_2(&_SaSocket, CMD_TRACE_ENABLE, sizeof(trace), &trace);
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
		if (_SaSocket!=INVALID_SOCKET) 
		{
			int ret=sok_send_2(&_SaSocket, CMD_STATUS_GET, 0, NULL);
		//	TrPrintfL(TRUE, "SetupAssist: CMD_STATUS_GET socket=%d, ret=%d", _SaSocket, ret);
			if (_Timeout>0 && --_Timeout==0) Error(ERR_CONT, 0, "Setup Assistant Scanner timeout");
		}
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
        case CMD_TRACE_GET:	 _do_sa_trace(socket, (SSetupAssist_TraceMsg*) msg); break;
		default:
		Error(ERR_CONT, 0, "Unknown Command 0x%04x, sent by Setup-Assist >>%s<<", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _do_sa_stat ---------------------------------------------------------------
static int _do_sa_stat(RX_SOCKET socket, SSetupAssist_StatusMsg	*pstat)
{
//	TrPrintfL(TRUE, "SetupAssist: Got Status");
	memcpy(&_Status, pstat, sizeof(_Status));
	_Status.hdr.msgId = REP_SETUP_ASSIST_STAT;
	_Status.motor.position = (INT32)(pstat->motor.position*_microns2steps);
	_Status.motor.stopPos  = (INT32)(pstat->motor.stopPos *_microns2steps);
	if (!_Status.motor.moving && _Timeout<MOVE_TIEMOUT) _Timeout=0;
	gui_send_msg(INVALID_SOCKET, &_Status);
	return REPLY_OK;
}

//--- _do_sa_trace ---------------------------------------------------------------
static int _do_sa_trace(RX_SOCKET socket, SSetupAssist_TraceMsg	*ptrace)
{
	TrPrintfL(TRUE, "SetupAssist: trace >>%s<<", ptrace->message);
	return REPLY_OK;
}

//--- sa_to_print_pos -------------------------------------------
void sa_to_print_pos(void)
{
	if (sa_connected()) 
	{
		sok_send_2(&_SaSocket, CMD_MOTOR_REFERENCE,    0, NULL);
		_Timeout = MOVE_TIEMOUT;
	}
}

//--- sa_in_print_pos ----
int sa_in_print_pos(void)
{
	if (!sa_connected()) return TRUE;
	return _Status.inputs & 0x01;
}

//--- sa_handle_gui_msg -------------------------------------------------------
void sa_handle_gui_msg(RX_SOCKET socket, void *pmsg_)
{
	SetupAssist_MoveCmd *pmsg = (SetupAssist_MoveCmd*)pmsg_;
	switch (pmsg->hdr.msgId)
	{
    case CMD_SA_REFERENCE:	//	Error(LOG, 0, "Send CMD_MOTOR_REFERENCE");
								sok_send_2(&_SaSocket, CMD_MOTOR_REFERENCE,    0, NULL); 
								break;

    case CMD_SA_STOP:		//	Error(LOG, 0, "Send CMD_SA_STOP");
								sok_send_2(&_SaSocket, CMD_MOTOR_STOP,    0, NULL); 
								break;

    case CMD_SA_MOVE:			// Error(LOG, 0, "Send CMD_MOTOR_MOVE");
								{
									SetupAssist_MoveCmd cmd;
									cmd.hdr.msgLen = sizeof(cmd);
									cmd.hdr.msgId  = CMD_MOTOR_MOVE;
									cmd.steps	   = (INT32)((double)pmsg->steps/_microns2steps);
									if (pmsg->speed) cmd.speed = pmsg->speed;
									else			 cmd.speed = 200;
									cmd.acc		   = 100;
									cmd.current	   = 300;
									sok_send(&_SaSocket, &cmd);
								}
								break;

    case CMD_SA_OUT_TRIGGER://	Error(LOG, 0, "Send CMD_SET_DENSIO_TRIGGER");
								{
									SetupAssist_OutTriggerCmd cmd;
									cmd.hdr.msgLen = sizeof(cmd);
									cmd.hdr.msgId  = CMD_OUT_TRIGGER;
									cmd.time_ms		  = 100;
									sok_send(&_SaSocket, &cmd);
								}
								break;

    case CMD_SA_WEB_MOVE:		// Error(LOG, 0, "Send CMD_SA_WEB_MOVE");
								plc_move_web(pmsg->steps, pmsg->speed);
								break;

    case CMD_SA_WEB_STOP:	//	gui_send_cmd(CMD_SA_WEB_STOP);
							//	Error(LOG, 0, "Send CMD_SA_WEB_STOP");
								plc_pause_printing(FALSE);
								break;

    default: Error(WARN, 0, "Unknown Command 0x%08x", pmsg->hdr.msgId);
	}
}