// ****************************************************************************
//
//	gui_svr.cpp
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include <time.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "enc_ctrl.h"
#include "gui_msg.h"
#include "gui_svr.h"

//--- Defines -----------------------------------------------------------------
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS	10
#define TIMEOUT			10000	// ms

//--- Externals ---------------------------------------------------------------


//--- Globals -----------------------------------------------------------------
typedef struct
{
	int time;
	int msgId;
} SGuiMsg;

#define GUI_MSG_CNT	64
static HANDLE	_HGuiSvr=NULL;

static int			_TimeoutCnt=0;
static RX_SOCKET	_Sockets[MAX_CONNECTIONS];
static int			_CheckSend[MAX_CONNECTIONS];
static int			_CheckRecv[MAX_CONNECTIONS];
static int			_CheckTime[MAX_CONNECTIONS];


static SGuiMsg	_GuiMsg[MAX_CONNECTIONS][GUI_MSG_CNT];
static int		_GuiMsgIn[MAX_CONNECTIONS];


//--- Prototypes ---------------------------------------------------------------
static int _gui_connected(RX_SOCKET socket, const char *peername);
static int _gui_closed(RX_SOCKET socket, const char *peername);

//--- gui_start ----------------------------------------------------------------
int	gui_start()
{
	for (int i=0; i<MAX_CONNECTIONS; i++) _Sockets[i]=INVALID_SOCKET;
	memset(_CheckRecv,	0, sizeof(_CheckRecv));
	memset(_CheckSend,	0, sizeof(_CheckSend));
	memset(_CheckTime,	0, sizeof(_CheckTime));
	memset(_GuiMsg,		0, sizeof(_GuiMsg));
	memset(_GuiMsgIn,	0, sizeof(_GuiMsgIn));
	sok_start_server(&_HGuiSvr, NULL, PORT_GUI, SOCK_STREAM, MAX_CONNECTIONS, handle_gui_msg, _gui_connected, _gui_closed);
	err_set_server(_HGuiSvr);
	return REPLY_OK;
}

//--- gui_end ---------------------------------------
int	gui_end(void)
{
	sok_stop_server(&_HGuiSvr);
	return REPLY_OK;
}

//--- _log ----------------------------------------------------------------
static void _log(int guiNo, int cmd, void *msg)
{
	SMsgHdr *phdr = (SMsgHdr*)msg;
	SGuiMsg *log  = &_GuiMsg[guiNo][_GuiMsgIn[guiNo]];
	log->time = rx_get_ticks();
	if (phdr==NULL) log->msgId = cmd;
	else			log->msgId = phdr->msgId;
	_GuiMsgIn[guiNo] = (_GuiMsgIn[guiNo]+1) % GUI_MSG_CNT;
}

//--- gui_send_cmd ------------------------------------------------------------
int gui_send_cmd (UINT32 cmd)
{
	return sok_send_to_clients_2(_HGuiSvr, NULL, cmd, 0, NULL);
}

//--- gui_recv -----------------------------------------
void gui_recv(RX_SOCKET socket)
{
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (socket==_Sockets[i])
		{
			_CheckRecv[i]++;
			_CheckTime[i]=rx_get_ticks();
			break;
		}
	}
}

//--- gui_send_msg -------------------------------------------------------------
int gui_send_msg(RX_SOCKET socket, void *msg)
{
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (socket==_Sockets[i])
		{
			_CheckSend[i]++;
			if (RX_Config.printer.type==printer_LH702) _log(i, 0, msg);
			break;
		}
	}

	return sok_send_to_clients(_HGuiSvr, &socket, msg);
}

//--- gui_send_msg_2 ---------------------------------------------------------
int gui_send_msg_2(RX_SOCKET socket, UINT32 cmd, int dataSize, void *data)
{
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (socket==_Sockets[i])
		{
			_CheckSend[i]++;
			if (RX_Config.printer.type==printer_LH702) _log(i, cmd, NULL);
			break;
		}
	}

	return sok_send_to_clients_2(_HGuiSvr, &socket, cmd, dataSize, data);	
}

//--- gui_send_log ------------------------------------------------------------
int gui_send_log(SLogItem *plog)
{
	return sok_send_to_clients_2(_HGuiSvr, NULL, EVT_GET_EVT, sizeof(*plog), plog);
}

//--- gui_send_print_queue -----------------------------------------------------
int gui_send_print_queue(UINT32 cmd, SPrintQueueItem *ppq)
{
	char *StateName[]={	"PQ_STATE_UNDEF", 
						"PQ_STATE_QUEUED", 
						"PQ_STATE_PREFLIGHT",
						"PQ_STATE_RIPPING",
						"PQ_STATE_SCREENING",
						"PQ_STATE_LOADING",
						"PQ_STATE_TRANSFER",
						"PQ_STATE_BUFFERED",
						"PQ_STATE_PRINTING",
						"PQ_STATE_PRINTED",
						"PQ_STATE_STOPPING",
						"PQ_STATE_STOPPED"};
	if (ppq->state==PQ_STATE_QUEUED) 
		ppq->state = ppq->state;
	
	if (ppq->state<SIZEOF(StateName)) 	TrPrintfL(TRUE, "id=%d, copy=%d, state=%s", ppq->id.id, ppq->id.copy, StateName[ppq->state]);
	else Error(ERR_CONT, 0, "Programming Error PrintQueue State=%d", ppq->state);
	return sok_send_to_clients_2(_HGuiSvr, NULL, cmd, sizeof(*ppq), ppq);
}

//--- gui_send_printer_status -------------------------------------------------------
int gui_send_printer_status(SPrinterStatus   *stat)
{
	return sok_send_to_clients_2(_HGuiSvr, NULL, EVT_PRINTER_STAT, sizeof(*stat), stat);	
}

//--- _gui_connected -------------------
static int _gui_connected(RX_SOCKET socket, const char *peername)
{
	TrPrintfL(TRUE, "GUI Connected >>%s<<", peername);
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (_Sockets[i]==INVALID_SOCKET)
		{
			_Sockets[i] = socket;
			_CheckRecv[i]=0;
			_CheckSend[i]=0;
			_CheckTime[i]=0;
			break;
		}
	}
	return REPLY_OK;
}


//--- _gui_closed -------------------
static int _gui_closed(RX_SOCKET socket, const char *peername)
{
	TrPrintfL(TRUE, "GUI Closed >>%s<<", peername);
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (socket==_Sockets[i])
		{
			_Sockets[i]=INVALID_SOCKET;
			_CheckRecv[i]=0;
			_CheckSend[i]=0;
			_CheckTime[i]=0;
			break;
		}
	}
	return REPLY_OK;
}

//--- gui_tick ----------------------
void gui_tick(void)
{
#ifndef DEBUG
	char str[MAX_PATH];
	int len = 0;
	int time=rx_get_ticks();
	str[0]=0;
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (_CheckTime[i] || _CheckRecv[i] || _Sockets[i]!=INVALID_SOCKET) 
		{
			len += sprintf(&str[len], "gui[%d](sok=%d, r=%03d, s=%03d)", i, _Sockets[i], _CheckRecv[i], _CheckSend[i]);
			if (_CheckTime[i] && time-_CheckTime[i]>TIMEOUT)
			{
				_TimeoutCnt++;
				Error(ERR_CONT, 0, "GUI Connection Timeout! (cnt=%d)", _TimeoutCnt);
				sok_close(&_Sockets[i]);
				int n=_GuiMsgIn[i];
				for (int m=0; m<GUI_MSG_CNT; m++)
				{
					TrPrintfL(TRUE, "GuiMsg[%d]: %d.%03d id=0x%08x", m, _GuiMsg[i][n].time/1000, _GuiMsg[i][n].time%1000, _GuiMsg[i][n].msgId);
					n=(n+1)%GUI_MSG_CNT;
				}
				_CheckTime[i]=0;
			}
		}
		_CheckRecv[i]=0;
		_CheckSend[i]=0;
	}

	if (RX_Config.printer.type==printer_LH702) TrPrintfL(TRUE, "GUI Check: %s, TimeoutCnt=%d, printState=%s, speed=%d", str, _TimeoutCnt, PrintStateStr[RX_PrinterStatus.printState], enc_speed());
#endif
}

//--- gui_test -------------------------------------------------------
void gui_test(void)
{
	for (int i=0; i<MAX_CONNECTIONS; i++)
	{
		if (_Sockets[i]!=INVALID_SOCKET && _CheckRecv[i]) 
		{
			TrPrintfL(TRUE, "gui[%d]: closed", i);
			sok_close(&_Sockets[i]);
			break;
		}
	}
}
