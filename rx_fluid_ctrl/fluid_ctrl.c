// ****************************************************************************
//
//	fluid_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "rx_fluid_ctrl.h"
#include "nios_fluid.h"
#include "daisy_chain.h"
#include "fpga_fluid.h"
#include "fluid_ctrl.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

#define MAX_CONNECTIONS	5

//--- buffer ---------------------------------------------------------
#define MSG_BUF_SIZE	512			// 256
#define MSG_MSG_SIZE	1024
typedef struct
{
	RX_SOCKET socket;
	UCHAR	  msg[MSG_MSG_SIZE];
} SFpgaMsg;

static SFpgaMsg _MsgBuf[MSG_BUF_SIZE];
static int		_MsgBufIn;
static int		_MsgBufOut;

//--- module globals -----------------------------------------------------------------
static HANDLE			_HServer;

static RX_SOCKET		_MainSocket;
// static UINT32			_PgCnt[MAX_HEADS_BOARD];
static int				_InkNo;
static int				_Connected=0;
static int				_Requests=0;
static int				_Replies=0;
static RX_SOCKET		_Socket = INVALID_SOCKET;

//--- prototypes ---------------------------------------------------------------------
static int _ctrl_connected  (RX_SOCKET socket, const char *peerName);
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName);
static int _save_ctrl_msg	(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);
static int _handle_ctrl_msg (RX_SOCKET socket, void *pmsg);

//--- command Handlers ---------------------------------------------------------------
static int _do_ping				(RX_SOCKET socket);
static int _do_fluid_stat		(RX_SOCKET socket, SHeadStateLight pressure[FLUID_BOARD_CNT]);
static int _do_fluid_ctrlMode	(RX_SOCKET socket, SFluidCtrlCmd *pmsg);
static int _do_set_purge_par	(RX_SOCKET socket, SPurgePar *ppar);
static void _do_scales_set_cfg	(RX_SOCKET socket, SScalesCfgMsg *pmsg);
static void _do_scales_get_cfg	(RX_SOCKET socket);
static void _do_scales_tara		(RX_SOCKET socket, INT32  *pmsg);
static void _do_scales_calib	(RX_SOCKET socket, SValue *pmsg);
static void _do_scales_stat		(RX_SOCKET socket);

//--- ctrl_init --------------------------------------------------------------------
int ctrl_init()
{
//	memset(&RX_EncoderStatus, 0, sizeof(RX_EncoderStatus));
	_MsgBufIn  = 0;
	_MsgBufOut = 0;

	sok_start_server(&_HServer, NULL, PORT_CTRL_FLUID, SOCK_STREAM, MAX_CONNECTIONS, _save_ctrl_msg, _ctrl_connected, _ctrl_deconnected);

	err_set_server(_HServer);
	
	return REPLY_OK;
}

//--- ctrl_end --------------------------------------------------------------------
int ctrl_end()
{
	return REPLY_OK;
}

//--- _ctrl_connected ---------------------------------------------------
static int _ctrl_connected (RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "connected from >>%s<<, socket=%d", peerName, socket);
	_Connected++;
	return REPLY_OK;
}

//--- _ctrl_deconnected
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "deconnected from >>%s<<, socket=%d", peerName, socket);
	_Connected--;
	return REPLY_OK;
}

//--- _save_ctrl_msg ---------------------------------------------------------
static int _save_ctrl_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;

//	TrPrintfL(1, "_save_ctrl_msg: Received msgId=0x%08x", phdr->msgId);

	if (phdr->msgId==0) return REPLY_OK;
	if (phdr->msgId == CMD_PING) _do_ping(socket);
	else
	{
		int idx;
		idx = (_MsgBufIn+1) % MSG_BUF_SIZE;
		if (idx==_MsgBufOut) 
			Error(ERR_CONT, 0, "TCP Message Buffer Overflow. _MsgBufIn=%d,  _MsgBufOut=%d, id=0x%08x", _MsgBufIn, _MsgBufOut, phdr->msgId);
		else
		{
			_MsgBuf[_MsgBufIn].socket = socket;
            if (len>sizeof(_MsgBuf[0].msg)) 
				Error(ERR_ABORT, 0, "Message Buffer Overflow, msgsize=%d, bufsize=%d", len, sizeof(_MsgBuf[0].msg));
			
			memcpy(_MsgBuf[_MsgBufIn].msg, pmsg, len);
			_MsgBufIn = idx;
//			fpga_signal_message();
		}
	}
	return REPLY_OK;
}


//--- ctrl_main -------------------
int  ctrl_main(int ticks, int menu)
{
	int cnt=0;
	while (_MsgBufOut!=_MsgBufIn)
	{
		cnt++;
		_handle_ctrl_msg(_MsgBuf[_MsgBufOut].socket, _MsgBuf[_MsgBufOut].msg);
		_MsgBufOut = (_MsgBufOut+1) % MSG_BUF_SIZE;
		break; // only one per call
	};
	return cnt;
}

//--- ctrl_connected ------------------------------
int ctrl_connected(void)
{
	return _Connected;
}

int ctrl_requests(void)
{
	return _Requests;
}

int ctrl_replies(void)
{
	return _Replies;
}

//--- _handle_ctrl_msg ---------------------------------------------------------
static int _handle_ctrl_msg(RX_SOCKET socket, void *msg)
{
	int reply = REPLY_ERROR;

	_Socket = socket; 
	
	//--- handle the message --------------
	reply = REPLY_OK;
	SMsgHdr *phdr = (SMsgHdr*)msg;
	void   *pdata =  &phdr[1];
//	TrPrintfL(1, "Received msgId=0x%08x", phdr->msgId);
	switch (phdr->msgId)
	{
	case CMD_ERROR_RESET:		nios_error_reset();											break;

    case CMD_FLUID_IS_CFG:		nois_set_is_cfg		((SInkSupplyCfg*)pdata);				break;

	case CMD_FLUID_CFG:			nios_set_cfg		((SFluidBoardCfg*)pdata);									
								fpga_cfg			();
								break;
                                        
	case CMD_FLUID_STAT:		_do_fluid_stat		(socket, (SHeadStateLight*)	pdata);		break;
	case CMD_FLUID_CTRL_MODE:	_do_fluid_ctrlMode	(socket, (SFluidCtrlCmd*)msg);			break;
    case CMD_FLUID_DEGASSER:	nios_set_degasser	((int*)pdata);							break;
	case CMD_SET_PURGE_PAR:		_do_set_purge_par	(socket, (SPurgePar*)	pdata);			break;

	case CMD_SCALES_SET_CFG:	 _do_scales_set_cfg(socket, (SScalesCfgMsg*)msg);			break;
	case CMD_SCALES_GET_CFG:	 _do_scales_get_cfg(socket);								break;
	case CMD_SCALES_TARA:		 _do_scales_tara(socket, (INT32*)pdata);					break;	
	case CMD_SCALES_CALIBRATE:	 _do_scales_calib(socket, (SValue*)pdata);					break;	
	case CMD_SCALES_STAT:		 _do_scales_stat(socket);									break;	
	default:		
					{
						char peer[64];
						sok_get_peer_name(socket, peer, NULL, NULL);
						Error(WARN, 0, "Unknown Command 0x%04x: len=%d peer >>%s<<", phdr->msgId, phdr->msgLen, peer);						
					}
					reply = REPLY_ERROR;
					break;
	}
	return reply;
};

//=== command  handlers ===========================================================================

//--- _do_ping -------------------------------------------------------------------------------------
static int _do_ping(RX_SOCKET socket)
{
	static int cnt=0;

	TrPrintf(1, "got CMD_PING");

	SMsgHdr msg;
	int len;
	msg.msgLen = sizeof(msg);
	msg.msgId  = REP_PING;
	len = send(socket, (char*)&msg, msg.msgLen, 0);
	TrPrintf(1, "Sent REP_PING, len=%d", len);
	/*
	switch(cnt%3)
	{
	case 0: 	Error(LOG,		100, "do PING cnt=%d, str>>%s<<", cnt, "Log-Entry vom Kopf"); 	break;
	case 1: 	Error(WARN,		100, "do PING cnt=%d, str>>%s<<", cnt, "Warnung Kopf"); 		break;
	case 2: 	Error(ERR_CONT, 100, "do PING cnt=%d, str>>%s<<", cnt, "Error vom Kopf"); 		break;
	}
	*/
	cnt++;
	return REPLY_OK;
}

//--- _do_fluid_stat ---------------------------------------------------------
static int _do_fluid_stat (RX_SOCKET socket, SHeadStateLight stat[FLUID_BOARD_CNT])
{
	int i;
	for (i=0;  i<FLUID_BOARD_CNT; i++) nios_set_head_state(i, &stat[i]);
	sok_send_2(&socket, REP_FLUID_STAT, sizeof(RX_FluidBoardStatus), &RX_FluidBoardStatus);
	if (daisy_chain_is_active())
	{
		SScalesStatMsg msg;
		msg.hdr.msgId  = REP_SCALES_STAT;
		msg.hdr.msgLen = sizeof(msg);
		daisy_chain_get_weight(msg.val, MAX_SCALES);
		sok_send(&socket, &msg);
	}
	
	return REPLY_OK;
}

//--- _do_fluid_ctrlMode --------------------------------------------
static int _do_fluid_ctrlMode	(RX_SOCKET socket, SFluidCtrlCmd *pmsg)
{
	nios_set_ctrlmode(pmsg->no, pmsg->ctrlMode);
	return REPLY_OK;
}

//--- _do_set_purge_par ---------------------------------------------
static int _do_set_purge_par(RX_SOCKET socket, SPurgePar *ppar)
{
	nios_set_purge_par(ppar->no, ppar->delay_pos_y, ppar->time);
	return REPLY_OK;
}

//--- _do_scales_set_cfg ---------------------------------------
static void _do_scales_set_cfg(RX_SOCKET socket, SScalesCfgMsg *pmsg)
{
	daisy_chain_set_tara(pmsg->tara, MAX_SCALES);
	daisy_chain_set_calib(pmsg->calib, MAX_SCALES);
}

//--- _do_scales_get_cfg ----------------------------------------
static void _do_scales_get_cfg(RX_SOCKET socket)
{
	SScalesCfgMsg msg;
	msg.hdr.msgId  = REP_SCALES_GET_CFG;
	msg.hdr.msgLen = sizeof(msg);
	daisy_chain_get_tara(msg.tara, MAX_SCALES);
	daisy_chain_get_calib(msg.calib, MAX_SCALES);
	sok_send(&socket, &msg);	
}

//--- _do_scales_tara ----------------------------------------------
static void _do_scales_tara(RX_SOCKET socket, INT32* no)
{
	daisy_chain_do_tara(*no);
	_do_scales_get_cfg(socket);		
}

//--- _do_scales_calib ----------------------------------------------
static void _do_scales_calib(RX_SOCKET socket, SValue* pmsg)
{
	daisy_chain_do_calib(pmsg);	
	_do_scales_get_cfg(socket);		
}

//--- _do_scales_stat ----------------------------------------------
static void _do_scales_stat		(RX_SOCKET socket)
{
	SScalesStatMsg msg;
	msg.hdr.msgId  = REP_SCALES_STAT;
	msg.hdr.msgLen = sizeof(msg);
	daisy_chain_get_weight(msg.val, MAX_SCALES);
	sok_send(&socket, &msg);				
}

//--- ctrl_socket -----------------------------------
RX_SOCKET ctrl_socket(void)
{
	return _Socket;			
}

