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
#include "setup_assist.h"


//--- statics ---------------------------------------------
static int			_SaThreadRunning = TRUE;
static RX_SOCKET	_SaSocket = INVALID_SOCKET;

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
	char buf[256];
	int i, ret;
	int err=FALSE;
	INT16 registers[8];
	
	while (_SaThreadRunning)
	{
		if (_SaSocket==INVALID_SOCKET)
		{
			ret = sok_open_client(&_SaSocket, SETUP_ASSIST_IP_ADDR, SETUP_ASSIST_PORT, SOCK_STREAM);
			if (ret==REPLY_OK)
			{
				char myaddr[32];
				TrPrintfL(1,"Setup Assist: Connected: myport=>>%s<<", sok_get_socket_name(_SaSocket,myaddr,NULL,NULL));
				sok_receiver(NULL, &_SaSocket, _sa_handle_msg, NULL);
				TrPrintfL(1,"Setup Assist: Connected: myport=>>%s<<", sok_get_socket_name(_SaSocket,myaddr,NULL,NULL));
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
		sok_send_2(&_SaSocket, CMD_STATUS_GET, 0, NULL);
		{
			SSetupAssist_StatusMsg msg;
			msg.header.msgLen = sizeof(msg);
			msg.header.msgId  = CMD_STATUS_GET;
			msg.inputs = 1;
			msg.motorHoldCurrent = 100;
			msg.motorMoveCurrent = 500;
			msg.powerStepStatus	 = 0x1234;
			msg.motorPosition	 = 1234;
			msg.motorVoltage	 = 234;
			msg.moving			 = FALSE;
			msg.refDone			 = FALSE;

			_sa_handle_msg(INVALID_SOCKET, &msg, msg.header.msgLen, NULL, NULL);
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
		default:
		Error(ERR_CONT, 0, "Unknown Command 0x%04x, sent by Setup-Assist >>%s<<", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _do_sa_stat ---------------------------------------------------------------
static int _do_sa_stat(RX_SOCKET socket, SSetupAssist_StatusMsg	*pstat)
{
	pstat->header.msgId = REP_SETUP_ASSIST_STAT;
	gui_send_msg(socket, pstat);
	return REPLY_OK;
}

//--- sa_handle_gui_msg -------------------------------------------------------
void sa_handle_gui_msg(RX_SOCKET socket, void *pmsg)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;
	switch (phdr->msgId)
	{
    case CMD_SA_REFERENCE:		Error(LOG, 0, "Send CMD_MOTOR_REFERENCE");
								sok_send_2(&_SaSocket, CMD_MOTOR_REFERENCE,    0, NULL); 
								break;
    case CMD_SA_TRIGGER_DENSIO:	Error(LOG, 0, "Send CMD_SET_DENSIO_TRIGGER");
								sok_send_2(&_SaSocket, CMD_SET_DENSIO_TRIGGER, 0, NULL); 
								break;
    case CMD_SA_MOVE:			Error(LOG, 0, "Send CMD_MOTOR_MOVE");								
								sok_send_2(&_SaSocket, CMD_MOTOR_MOVE,		   &phdr[1], phdr->msgLen-sizeof(SMsgHdr)); 
								break;
    default: Error(WARN, 0, "Unknown Command 0x%08x", phdr->msgId);
	}
}