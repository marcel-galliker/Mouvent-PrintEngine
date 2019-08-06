// ****************************************************************************
//
//	enc_ctrl.c
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
#include "fpga_enc.h"
#include "tw8.h"
#include "enc_ctrl.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

#define MAX_CONNECTIONS	5
#define MSG_BUF_SIZE	256
#define MSG_MSG_SIZE	128

typedef struct
{
	RX_SOCKET socket;
	UCHAR	  msg[MSG_MSG_SIZE];
} SFpgaMsg;

//--- module globals -----------------------------------------------------------------
static HANDLE			_HServer;

static RX_SOCKET		_MainSocket;
// static UINT32			_PgCnt[MAX_HEADS_BOARD];
static int				_InkNo;
static int				_Connected=0;
static int				_Requests=0;
static int				_Replies=0;
static int				_PgNo=0;

static SFpgaMsg _MsgBuf[MSG_BUF_SIZE];
static int		_MsgBufIn;
static int		_MsgBufOut;

static SEncoderPgDist	_DistMsg[MSG_BUF_SIZE];

//--- prototypes ---------------------------------------------------------------------
static int _ctrl_connected  (RX_SOCKET socket, const char *peerName);
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName);
static int _save_ctrl_msg   (RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);
static int _handle_ctrl_msg (RX_SOCKET socket, void *pmsg);

//--- command Handlers ---------------------------------------------------------------
static int _do_ping			(RX_SOCKET socket);
static int _do_encoder_cfg 	(RX_SOCKET socket, SEncoderCfg 	*cfg);
static int _do_encoder_pg_dist(RX_SOCKET socket, SEncoderPgDist *dist);
static int _do_encoder_pg_restart(RX_SOCKET socket);
static int _do_encoder_stat (RX_SOCKET socket);
static int _do_simu_encoder (RX_SOCKET socket, int *pkhz);
static int _do_stop_printing(RX_SOCKET socket);

//--- ctrl_init --------------------------------------------------------------------
int ctrl_init()
{
	int i;
	
	_MsgBufIn  = 0;
	_MsgBufOut = 0;

	memset(&RX_EncoderStatus, 0, sizeof(RX_EncoderStatus));
	RX_EncoderStatus.ampl_old	= 0;
	RX_EncoderStatus.ampl_new   = 0;
	RX_EncoderStatus.percentage = 0;
	for (i=0; i<SIZEOF(RX_EncoderStatus.corrRotPar); i++)
		RX_EncoderStatus.corrRotPar[i] = INVALID_VALUE;	

	sok_start_server(&_HServer, NULL, PORT_CTRL_ENCODER, SOCK_STREAM, MAX_CONNECTIONS, _save_ctrl_msg, _ctrl_connected, _ctrl_deconnected);

	err_set_server(_HServer);
	
	return REPLY_OK;
}

//--- ctrl_end --------------------------------------------------------------------
int ctrl_end()
{
	return REPLY_OK;
}

//--- ctrl_simu ---------------------------------------
void ctrl_simu(void)
{
	// simulates start of 10 products with fixed gap

	int i;
	int freq = 7;
	RX_SOCKET socket;
	socket = INVALID_SOCKET;
	
	SEncoderCfg cfg;
	cfg.orientation = FALSE;
	cfg.incPerMeter = 1000000;
	cfg.pos_pg_fwd  = 10000; 
	cfg.pos_pg_bwd  = 10000000;
	cfg.pos_actual  = 0;
	cfg.correction  = 0;
	cfg.scanning	= FALSE;

	SEncoderPgDist dist;
	dist.cnt    = 1;
	dist.dist   = 100000;
	dist.ignore = 0;
	dist.window = 0;
	
	fpga_pg_init(FALSE);
	_do_encoder_cfg(socket, &cfg);
	for (i=0; i<10; i++) _do_encoder_pg_dist(socket, &dist);	
	_do_simu_encoder(socket, &freq);
}

//--- _ctrl_connected ---------------------------------------------------
static int _ctrl_connected (RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "connected from >>%s<<, socket=%d", peerName, socket);
	_Connected++;
	return REPLY_OK;
}

//--- _ctrl_deconnected ----------------------------------------------
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "deconnected from >>%s<<, socket=%d", peerName, socket);
	_Connected--;
	return REPLY_OK;
}

//--- ctrl_connected ------------------------------
int ctrl_connected(void)
{
	return _Connected;
}

//--- ctrl_requests ---------------------------------
int ctrl_requests(void)
{
	return _Requests;
}

//--- ctrl_replies ---------------------------------
int ctrl_replies(void)
{
	return _Replies;
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
	//	return 1;
	};
	return cnt;
}

//--- _handle_ctrl_msg ---------------------------------------------------------
static int _handle_ctrl_msg(RX_SOCKET socket, void *pmsg)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;
	int reply = REPLY_OK;;

	//--- handle the message --------------
	switch (phdr->msgId)
	{
	case CMD_ENCODER_CFG:		_do_encoder_cfg	(socket, (SEncoderCfg*) &phdr[1]);		break;
	case CMD_ENCODER_PG_INIT:	_PgNo = 0;
								fpga_pg_init(FALSE);		
								break;
	case CMD_ERROR_RESET:		tw8_reset_error(); break;
	case CMD_ENCODER_PG_DIST:	_do_encoder_pg_dist(socket, (SEncoderPgDist*)&phdr[1]);	break;
	case CMD_ENCODER_PG_STOP:	fpga_pg_stop();											break;
	case CMD_ENCODER_PG_RESTART:_do_encoder_pg_restart(socket);							break;
	case CMD_ENCODER_STAT:		_do_encoder_stat(socket);								break;
	case CMD_FPGA_SIMU_ENCODER:	_do_simu_encoder(socket, (int*)&phdr[1]);				break;
	case CMD_STOP_PRINTING:		fpga_stop_printing();									break;
	case CMD_ABORT_PRINTING:	fpga_abort_printing();									break;
	case CMD_ENCODER_UV_ON:		fpga_uv_on();
								_do_encoder_stat(socket);
								break;
	case CMD_ENCODER_UV_OFF:	fpga_uv_off();											
								_do_encoder_stat(socket);
								break;
	case CMD_ENCODER_DISABLE:	fpga_encoder_enable(FALSE);	break;
	case CMD_ENCODER_ENABLE:	fpga_encoder_enable(TRUE);	break;
	default:		Error(LOG, 0, "Unknown Command 0x%04x", phdr->msgId);
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

//--- _do_encoder_cfg ---------------------------------------------------------
static int _do_encoder_cfg 	(RX_SOCKET socket, SEncoderCfg 	*pcfg)
{
	SReply reply;

	_Requests++;
	memcpy(&RX_EncoderCfg, pcfg, sizeof(RX_EncoderCfg));
	fpga_enc_config(0, pcfg, FALSE);
	reply.hdr.msgId  = REP_ENCODER_CFG;
	reply.hdr.msgLen = sizeof(reply);
	reply.reply = fpga_pg_config(socket, pcfg, FALSE);
	if (reply.reply==REPLY_OK) _Replies++;
	sok_send(&socket, &reply);
	return REPLY_OK;
}

//--- _do_encoder_pg_dist ------------------------------------------------------
static int _do_encoder_pg_dist(RX_SOCKET socket, SEncoderPgDist *pmsg)
{
	int time=rx_get_ticks();
	static int _time=0;
	TrPrintfL(TRUE, "_do_encoder_pg_dist(no=%d, cnt=%d, dist=%d) time=%d", ++_PgNo, pmsg->cnt, pmsg->dist, time-_time);
	_time=time;
	memcpy(&_DistMsg[_PgNo%SIZEOF(_DistMsg)], pmsg, sizeof(SEncoderPgDist)); 
	if (pmsg->dist)		fpga_pg_set_dist(pmsg->cnt, pmsg->dist);
	if (pmsg->ignore)	Error(ERR_CONT, 0, "Not implemented yet");
	if (pmsg->window)	Error(ERR_CONT, 0, "Not implemented yet");
	return REPLY_OK;
}

//--- _do_encoder_pg_restart -------------------------------
static int _do_encoder_pg_restart(RX_SOCKET socket)
{
	int i;
	SEncoderPgDist *pmsg;
	
	TrPrintfL(TRUE, "_do_encoder_pg_restart");
	
	fpga_pg_init(TRUE);
	
	//--- refill FIFO -------------------
	for (i=RX_EncoderStatus.PG_cnt; i<_PgNo; i++)
	{
		pmsg = &_DistMsg[i%SIZEOF(_DistMsg)];
		TrPrintfL(TRUE, "_do_encoder_pg_dist(no=%d, cnt=%d, dist=%d)", i, pmsg->cnt, pmsg->dist);
		if (pmsg->dist)		fpga_pg_set_dist(pmsg->cnt, pmsg->dist);
		if (pmsg->ignore)	Error(ERR_CONT, 0, "Not implemented yet");
		if (pmsg->window)	Error(ERR_CONT, 0, "Not implemented yet");									
	}

	fpga_enc_config(0, &RX_EncoderCfg, TRUE);
	fpga_pg_config(INVALID_SOCKET, &RX_EncoderCfg, TRUE);
	
	return REPLY_OK;
}

//--- _do_simu_encoder -----------------------------------------------
static int _do_simu_encoder (RX_SOCKET socket, int *pkhz)
{
	fpga_enc_simu(*pkhz);
	return REPLY_OK;
}

//--- _do_encoder_stat ---------------------------------------------------------
static int _do_encoder_stat (RX_SOCKET socket)
{
	RX_EncoderStatus.info.analog_encoder = tw8_present();
	sok_send_2(&socket, REP_ENCODER_STAT, sizeof(RX_EncoderStatus), &RX_EncoderStatus);
	return REPLY_OK;
}
