// ****************************************************************************
//
//	stepper_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
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
#include "args.h"
#include "cln.h"
#include "fpga_stepper.h"
#include "lb701.h"
#include "lb702.h"
#include "lbrob.h"
#include "tx801.h"
#include "tx404.h"
#include "tx80x_wd.h"
#include "dp803.h"
#include "txrob.h"
#include "slide.h"
#include "sa.h"
#include "tcp_ip.h"
#include "test_table.h"
#include "cleaf.h"
#include "tts_lift.h"
#include "tts_ink.h"
#include "stepper_ctrl.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

#define MAX_CONNECTIONS	5
#define MSG_BUF_SIZE	2048	//512	//256
#define MSG_MSG_SIZE	2048	//512

static RX_SOCKET	_MainSocket=INVALID_SOCKET;

//--- buffer ---------------------------------------------------------
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
static int				_Connections=0;

// static UINT32			_PgCnt[MAX_HEADS_BOARD];

//--- prototypes ---------------------------------------------------------------------
static int _ctrl_connected  (RX_SOCKET socket, const char *peerName);
static int _ctrl_deconnected(RX_SOCKET socket, const char *peerName);
static int _handle_ctrl_msg (RX_SOCKET socket, void *pmsg);
static int _save_ctrl_msg   (RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);

//--- command Handlers ---------------------------------------------------------------
static int _do_ping				(RX_SOCKET socket);
static void _do_config			(SStepperCfg *pcfg);

//--- ctrl_init --------------------------------------------------------------------
int ctrl_init()
{
    int errNo = 0;
    _MsgBufIn  = 0;
	_MsgBufOut = 0;

    system("netstat -lt > /tmp/ports_1");

    errNo = sok_start_server(&_HServer, NULL, PORT_CTRL_STEPPER, SOCK_STREAM, MAX_CONNECTIONS, _save_ctrl_msg, _ctrl_connected, _ctrl_deconnected);
	TrPrintfL(1, "Fehler %d", errNo);

    system("netstat -lt > /tmp/ports_2");
    
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
	if (_Connections==0) fpga_connected();
	_Connections++;
	return REPLY_OK;
}

//--- ctrl_send_2 --------------------
int ctrl_send_2(INT32 id, UINT32 dataLen, void *data)
{
	return sok_send_2(&_MainSocket, id, dataLen, data);
}

//--- _ctrl_deconnected ---------------------------------------------------
static int _ctrl_deconnected (RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "connection from >>%s<<, socket=%d closed", peerName, socket);
	_Connections--;
	if (_Connections==0) fpga_deconnected();
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
    int errNo = 0;
    if (!_HServer)
    {
        errNo = sok_start_server(&_HServer, NULL, PORT_CTRL_STEPPER, SOCK_STREAM, MAX_CONNECTIONS, _save_ctrl_msg, _ctrl_connected, _ctrl_deconnected);
		TrPrintfL(1, "Fehler %d", errNo);
    }
    while (_MsgBufOut!=_MsgBufIn)
	{
		cnt++;
		_handle_ctrl_msg(_MsgBuf[_MsgBufOut].socket, _MsgBuf[_MsgBufOut].msg);
		_MsgBufOut = (_MsgBufOut+1) % MSG_BUF_SIZE;
		break; // only one per call
	};
	return cnt;
}

//--- ctrl_cmd_name ----------------------------------------
char *ctrl_cmd_name(int cmd)
{
	static char str[32];
	switch (cmd)
	{
	case CMD_LIFT_REFERENCE:	strcpy(str, "Reference");	break; 
	case CMD_LIFT_PRINT_POS:	strcpy(str, "Goto Print");	break; 
	case CMD_LIFT_UP_POS:		strcpy(str, "Go UP");		break; 
	case CMD_LIFT_CAPPING_POS:	strcpy(str, "Capping");		break; 
	default:  					sprintf(str, "0x%08x", cmd);		
	}
	return str;
}

//--- _handle_ctrl_msg ---------------------------------------------------------
int _handle_ctrl_msg(RX_SOCKET socket, void *pmsg)//, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;
    //--- handle the message --------------
	reply = REPLY_OK;
	SMsgHdr *phdr = (SMsgHdr*)pmsg;

    if (socket != INVALID_SOCKET) _MainSocket = socket;

    switch (phdr->msgId)
	{
	case CMD_STEPPER_CFG:			_do_config((SStepperCfg*)&phdr[1]);	break;
	case CMD_STEPPER_STAT:		//	sok_send_2(&socket, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);	
									break;

//	case CMD_PING:					_do_ping			(socket);							 break;
	default:
		switch (RX_StepperCfg.printerType)
		{
		case printer_test_table:		tt_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]); break;
        case printer_test_table_seon:	if (RX_StepperCfg.boardNo == 0)
										    tts_lift_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										else
                                            tts_ink_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										break;

		case printer_TX801:				
		case printer_TX802:				if (RX_StepperCfg.boardNo == step_lift) tx801_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										else
										{
										    txrob_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										    if (!RX_StepperStatus.robinfo.wd_unused) tx80x_wd_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										} 
										break;
        case printer_TX404:				if (RX_StepperCfg.boardNo == step_lift) tx404_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										else
										{
										    txrob_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										    if (!RX_StepperStatus.robinfo.wd_unused) tx80x_wd_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										} 
										break;
			
		case printer_cleaf:				cleaf_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]); break;
			
		case printer_test_slide:		break;
		case printer_test_slide_only:	break;
		case printer_test_slide_HB:		break;
        case printer_Dropwatcher:		break;
			
		case printer_LB701:				lb701_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);	break;
		case printer_LB702_UV:
		case printer_LB702_WB:			
		case printer_LH702:				if (RX_StepperStatus.robot_used) lbrob_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);
										else							 lb702_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);	
										break;

        case printer_setup_assist:		sa_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);	break;

//		case printer_LBROB:				lbrob_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);	break;
			
		case printer_DP803:				dp803_handle_ctrl_msg(socket, phdr->msgId, &phdr[1]);	break;
			
		default:	Error(ERR_CONT, 0, "PrinterType %d not implemented, msgId=0x%08x", RX_StepperCfg.printerType, phdr->msgId);
		}			
	}
	sok_send_2(&socket, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
	return reply;
};

//=== command  handlers ===========================================================================

//--- _do_ping -------------------------------------------------------------------------------------
static int _do_ping(RX_SOCKET socket)
{
	TrPrintf(1, "got CMD_PING");
	sok_send_2(&socket, REP_PING, 0, NULL);
	return REPLY_OK;
}

//--- _do_config -----------------------------------------------------------------------------
static void _do_config(SStepperCfg *pcfg)
{	
	memcpy(&RX_StepperCfg, pcfg, sizeof(RX_StepperCfg));

    RX_StepperStatus.no = RX_StepperCfg.boardNo;

	switch (RX_StepperCfg.printerType)
	{
	case printer_test_slide:		break;
	case printer_test_slide_only:	break;
	case printer_test_slide_HB:		break;
    case printer_Dropwatcher:		break;
	case printer_test_table:	tt_init(); break;
    case printer_test_table_seon:	if (RX_StepperCfg.boardNo == 0)
									    tts_lift_init();
									else
                                        tts_ink_init();
									break;
	case printer_TX801:			
	case printer_TX802:			if (RX_StepperCfg.boardNo == step_lift) tx801_init();
								else
								{
								    txrob_init();
								    if (!RX_StepperStatus.robinfo.wd_unused) tx80x_wd_init();
								}
								break;
		
	case printer_TX404:			if (RX_StepperCfg.boardNo == step_lift) tx404_init();
								else
								{
								    txrob_init();
								    if (!RX_StepperStatus.robinfo.wd_unused) tx80x_wd_init();
								}
								break;
		
	case printer_cleaf:			cleaf_init();	break;
		
	case printer_LB701:			lb701_init();	break;
		
	case printer_LB702_UV:			
	case printer_LB702_WB:		
	case printer_LH702:			lb702_init();	break;
    case printer_setup_assist:	sa_init(); break;
//	case printer_LBROB:			lbrob_init();	break;
		
	case printer_DP803:			dp803_init();	break;
		

	default:	Error(ERR_CONT, 0, "PrinterType %d not implemented", RX_StepperCfg.printerType);
	}						
}
