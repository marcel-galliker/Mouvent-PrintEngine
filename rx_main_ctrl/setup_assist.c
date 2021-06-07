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

static SStepperStat	_Status;

//--- prorotypes -----------------------------------------
static void *_sa_thread(void *lpParameter);
static void _sa_set_config(void);
static int  _sa_socket_closed(RX_SOCKET socket, const char *peerName);
static void *_sa_tick_thread(void *lpParameter);
static int _sa_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _do_sa_stat(RX_SOCKET socket, SStepperStat *pstat);

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
			if (sok_open_client_2(&_SaSocket, SETUP_ASSIST_IP_ADDR, PORT_CTRL_STEPPER, SOCK_STREAM, _sa_handle_msg, _sa_socket_closed)==REPLY_OK)
			{
                Error(LOG, 0, "Connected");
                _sa_set_config();
			}
		}
		
		rx_sleep(1000);
	}
	return NULL;
}

//--- _sa_socket_closed -------------------------
static int  _sa_socket_closed(RX_SOCKET socket, const char *peerName)
{
	Error(ERR_CONT, 0, "Socket %d closed", socket);
	_SaSocket = INVALID_SOCKET;
}

//--- _sa_set_config -------------------
static void _sa_set_config(void)				
{
	SStepperCfg cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.printerType = printer_setup_assist;
	sok_send_2(&_SaSocket, CMD_STEPPER_CFG, sizeof(cfg), &cfg);
}

//--- _sa_tick_thread ---------------------------------------------
static void* _sa_tick_thread(void *lpParameter)
{
	while (_SaThreadRunning)
	{
		if (_SaSocket!=INVALID_SOCKET) 
		{
			int ret=sok_send_2(&_SaSocket, CMD_STEPPER_STAT, 0, NULL);
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
	void *data = (void*)&phdr[1];
	_Timeout = MOVE_TIEMOUT;
	switch(phdr->msgId)
	{
		case REP_STEPPER_STAT: _do_sa_stat(socket, (SStepperStat*) data); break;
        case EVT_GET_EVT:		SlaveError(dev_stepper, 99, &((SLogMsg*)msg)->log);	
								break;
		default:
		Error(ERR_CONT, 0, "Unknown Command 0x%04x, sent by Setup-Assist", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _do_sa_stat ---------------------------------------------------------------
static int _do_sa_stat(RX_SOCKET socket, SStepperStat *pstat)
{
//	TrPrintfL(TRUE, "SetupAssist: Got Status");
	memcpy(&_Status, pstat, sizeof(_Status));
	gui_send_msg_2(0, REP_SETUP_ASSIST_STAT, sizeof(_Status), &_Status);
	return REPLY_OK;
}

//--- sa_to_print_pos -------------------------------------------
void sa_to_print_pos(void)
{
	if (sa_connected()) 
	{
		sok_send_2(&_SaSocket, CMD_SA_REFERENCE,    0, NULL);
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
    case CMD_SA_REFERENCE:
    case CMD_SA_STOP:			
    case CMD_SA_MOVE:			
    case CMD_SA_UP:
    case CMD_SA_DOWN:
								sok_send(&_SaSocket, pmsg_);
								break;

    case CMD_SA_WEB_MOVE:		// Error(LOG, 0, "Send CMD_SA_WEB_MOVE");
								plc_move_web(pmsg->steps, 1);
								break;

    case CMD_SA_WEB_STOP:	//	gui_send_cmd(CMD_SA_WEB_STOP);
							//	Error(LOG, 0, "Send CMD_SA_WEB_STOP");
								plc_pause_printing(FALSE);
								break;

    default: Error(WARN, 0, "Unknown Command 0x%08x", pmsg->hdr.msgId);
	}
}