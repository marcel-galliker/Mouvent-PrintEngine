// ****************************************************************************
//
//	TcpSrv.c
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
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "CtrlSrv.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

//--- module globals -----------------------------------------------------------------
static HANDLE	sHServer;
static INT32  sLen=0;
static INT32  sId=0;
static UINT32 sTime0=0;
static UINT32 sErrCnt=0;
UCHAR  sBuffer[0x10000];

//--- prototypes ---------------------------------------------------------------------
static int handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);

//--- command Handlers ---------------------------------------------------------------

//--- DataSrv_init --------------------------------------------------------------------
int DataSrv_init()
{
	memset(sBuffer, 0, sizeof(sBuffer));
	SOK_start_server(&sHServer, NULL, PORT_UDP_DATA, SOCK_DGRAM, 0, handle_msg);

	return REPLY_OK;
}

//--- DataSrv_end --------------------------------------------------------------------
int DataSrv_end()
{
	return REPLY_OK;
}

//--- handle_msg ---------------------------------------------------------

static int handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par)
{
	INT32 *pid = (INT32*)msg;

	TrPrintfL(0,"handle_msg, len=%d, id=%ld", len, *pid);

	if (*pid==-1)
	{
		if (sId)
		{
			UINT32 time = RX_get_ticks()-sTime0;
			TrPrintfL(1, "Transfer capacity: %d Bytes in %d ms, errCnt=%ld", sLen, time, sErrCnt);
			TrPrintfL(1, "Transfer capacity: %f MB/Sec", 1000.0*sLen/1024.0/1024.0/time);
			sTime0=0;
			sLen=0;
			sId =0;
			sErrCnt=0;
		}
	}
	else
	{
		sBuffer[*pid]++;
		if (FALSE && sLen && *pid%1000==0)
		{
			UINT32 time = RX_get_ticks()-sTime0;
			TrPrintfL(1, "Transfer capacity: id=%ld: %d Bytes in %d ms", *pid, sLen, time);
			TrPrintfL(1, "Transfer capacity: %f MB/Sec", 1000.0*sLen/1024.0/1024.0/time);
		}
		if (sTime0==0) sTime0=RX_get_ticks();
		if (*pid!=sId) sErrCnt++; // Error(ERR, 0, "ID mismatch: expedted=%d, received=%d", sId, *pid);
		sId = *pid+1;
		sLen += len-4;
	}

	return REPLY_OK;
};
