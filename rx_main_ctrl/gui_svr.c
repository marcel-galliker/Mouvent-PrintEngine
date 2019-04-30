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
#include "gui_msg.h"
#include "gui_svr.h"

//--- Defines -----------------------------------------------------------------
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS	10

//--- Externals ---------------------------------------------------------------


//--- Globals -----------------------------------------------------------------

HANDLE	_HGuiSvr=NULL;

//--- Prototypes ---------------------------------------------------------------
static int _gui_connected(RX_SOCKET socket, const char *peername);
static int _gui_closed(RX_SOCKET socket, const char *peername);

//--- gui_start ----------------------------------------------------------------
int	gui_start()
{
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

//--- gui_send_cmd ------------------------------------------------------------
int gui_send_cmd (UINT32 cmd)
{
	return sok_send_to_clients_2(_HGuiSvr, cmd, 0, NULL);
}

//--- gui_send_msg -------------------------------------------------------------
int gui_send_msg(RX_SOCKET socket, void *msg)
{
	if (socket && socket!=INVALID_SOCKET) return sok_send(&socket, msg); 
	else        return sok_send_to_clients(_HGuiSvr, msg);
}

//--- gui_send_msg_2 ---------------------------------------------------------
int gui_send_msg_2(RX_SOCKET socket, UINT32 cmd, int dataSize, void *data)
{
	if (socket!=0 && socket!=INVALID_SOCKET) return sok_send_2(&socket, INADDR_ANY, cmd, dataSize, data);
	else                                        return sok_send_to_clients_2(_HGuiSvr, cmd, dataSize, data);	
}

//--- gui_send_log ------------------------------------------------------------
int gui_send_log(SLogItem *plog)
{
	return sok_send_to_clients_2(_HGuiSvr, EVT_GET_EVT, sizeof(*plog), plog);
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
	return sok_send_to_clients_2(_HGuiSvr, cmd, sizeof(*ppq), ppq);
}

//--- gui_send_printer_status -------------------------------------------------------
int gui_send_printer_status(SPrinterStatus   *stat)
{
	return sok_send_to_clients_2(_HGuiSvr, EVT_PRINTER_STAT, sizeof(*stat), stat);	
}

//--- _gui_connected -------------------
static int _gui_connected(RX_SOCKET socket, const char *peername)
{
	TrPrintfL(TRUE, "GUI Connected >>%s<<", peername);
	return REPLY_OK;
}

//--- _gui_closed -------------------
static int _gui_closed(RX_SOCKET socket, const char *peername)
{
	TrPrintfL(TRUE, "GUI Closed >>%s<<", peername);
	return REPLY_OK;
}