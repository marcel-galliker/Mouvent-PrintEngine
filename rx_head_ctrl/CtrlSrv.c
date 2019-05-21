// ****************************************************************************
//
//	CtrlSrv.c
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
#include "tcp_ip.h"
#include "fpga.h"
#include "fpga_simu.h"
#include "CtrlSrv.h"

#ifdef linux
	#include <sys/socket.h>
	#define SOCKET int
#endif

#define MAX_CONNECTIONS	5

//--- module globals -----------------------------------------------------------------
static HANDLE			HServer;
static UINT32			*Data=NULL;
static int				size = 256*1024*1024;

static RX_SOCKET		sMainSocket;
static UINT32			sPgCnt[MAX_HEADS_BOARD];
// static UINT32		sPdCnt[MAX_HEADS_BOARD];
static UINT32			sPdCnt;
static SPageId			sPageId[256];

//--- prototypes ---------------------------------------------------------------------
static void *ctrl_thread(void *par);
static int  _ctrl_thread_running; 
static int handle_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);

//--- command Handlers ---------------------------------------------------------------
static int do_ping				(RX_SOCKET socket);
static int do_block_used		(RX_SOCKET socket, SBlockUsedCmd 	*msg);
static int do_fpga_image		(RX_SOCKET socket, SFpgaImageCmd 	*msg);
static int do_simu_print		(RX_SOCKET socket);
static int do_write_image	 	(RX_SOCKET socket, SFpgaWriteBmpCmd *msg);
static int do_head_board_cfg 	(RX_SOCKET socket, SHeadBoardCfg 	*cfg);

//--- CtrlSrv_init --------------------------------------------------------------------
int CtrlSrv_init()
{

//	sok_start_server(&HServer, NULL, PORT_CTRL_HEAD, SOCK_STREAM, MAX_CONNECTIONS, handle_ctrl_msg, NULL, NULL);


//	err_set_server(HServer);
	Data = (UINT32*)malloc(size);
	memset(Data, 0, sizeof(Data));
	
	_ctrl_thread_running = TRUE;
	rx_thread_start(ctrl_thread, NULL, 0, "ctrl_thread");

	return REPLY_OK;
}

//--- CtrlSrv_end --------------------------------------------------------------------
int CtrlSrv_end()
{
	_ctrl_thread_running = FALSE;
	return REPLY_OK;
}

//--- ctrl_thread -------------------------------------------------
static void *ctrl_thread(void *par)
{
	RX_SOCKET client;
	while (_ctrl_thread_running)
	{
		sok_open_client(&client, RX_CTRL_MAIN, PORT_CTRL_MAIN, SOCK_STREAM);
		TrPrintfL(TRUE, "connected to rx_main_ctrl");
		err_set_client(client);
		sok_receiver(NULL, client, handle_ctrl_msg, NULL);
		err_set_client(NULL);
		TrPrintfL(TRUE, "disconnected from rx_main_ctrl");
	}
}

//--- handle_ctrl_msg ---------------------------------------------------------
static int handle_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	int reply = REPLY_ERROR;

	//--- handle the message --------------
	reply = REPLY_OK;
	SMsgHdr *phdr = (SMsgHdr*)msg;
//	TrPrintf(1, "Received msgId=0x%08x", phdr->msgId);
	switch (phdr->msgId)
	{
	case CMD_PING:				do_ping			 (socket);								break;
	case CMD_FPGA_IMAGE:		do_fpga_image	 (socket, (SFpgaImageCmd*)		msg);	break;
	case CMD_FPGA_WRITE_BMP:	do_write_image	 (socket, (SFpgaWriteBmpCmd*) 	msg);	break;
	case CMD_FPGA_SIMU_PRINT:	do_simu_print	 (socket);								break;
	case CMD_GET_BLOCK_USED:	do_block_used	 (socket, (SBlockUsedCmd*)		msg);	break;
	case CMD_HEAD_BOARD_CFG:	do_head_board_cfg(socket, (SHeadBoardCfg*)&phdr[1]);	break;
	default:		TrPrintf(1, "Unknown Command 0x%04x", phdr->msgId);
					reply = REPLY_ERROR;
					break;
	}
	{
		int head;
		int cnt, done;
		SPrintFileMsg	msg;
		while (1)
		{
			for (head=0, cnt=0, done=0; cnt==done && head<MAX_HEADS_BOARD;  head++)
			{
				if (RX_HBConfig.head[head].blockCnt) 
				{
					cnt++;
					if (Fpga.stat->pdCnt[head] > sPdCnt) done++;
				}
			}

			if (cnt && cnt==done)
			{
				//--- print done for all heads --------------
				msg.hdr.msgLen  = sizeof(msg);
				msg.hdr.msgId	= EVT_PRINT_FILE;
				memcpy(&msg.id, &sPageId[sPdCnt&0xff], sizeof(msg.id));
				msg.evt			= DATA_PRINT_DONE;
				msg.bufReady	= 0;
				sPdCnt++;
				sok_send(sMainSocket, &msg);
				TrPrintfL(TRUE, "PRINT DONE:  id=%d, page=%d, copy=%d, scan=%d", msg.id.id, msg.id.page, msg.id.copy, msg.id.scan);
			}
			else break;
		};
	}
	return reply;
};

//=== command  handlers ===========================================================================

//--- do_ping -------------------------------------------------------------------------------------
static int do_ping(RX_SOCKET socket)
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

//--- do_block_used ----------------------------------------------------------
static int do_block_used	(RX_SOCKET socket, SBlockUsedCmd *msg)
{
	SBlockUsedRep 	reply;
	UINT32			min, max;

	TrPrintfL(TRUE, "do_block_used id=%d, blockIdx=%d, blkCnt=%d (bits %d .. %d)", msg->id, msg->blockIdx, msg->blkCnt, msg->blockIdx*8, (msg->blockIdx+msg->blkCnt)*8);
	simu_blockCnt = 0;

	min=RX_HBConfig.head[msg->headNo].blockNo0/8;
	max=min+RX_HBConfig.head[msg->headNo].blockCnt/8;

	if (msg->blockIdx>=min && msg->blockIdx<max)
	{
		reply.hdr.msgId = REP_GET_BLOCK_USED;
		reply.headNo    = msg->headNo;
		reply.id		= msg->id;
		reply.blockIdx	= msg->blockIdx;

		if (msg->blkCnt > max-min) reply.blkCnt = max-min;
		else                       reply.blkCnt = msg->blkCnt;
			
		if (reply.blockIdx+reply.blkCnt<max)
		{
	//		TrPrintfL(TRUE, "REP_GET_BLOCK_USED idx=%d, cnt=%d", reply.blockIdx*8, reply.blkCnt*8);
			memcpy(reply.used,   Fpga.blockUsed+reply.blockIdx, reply.blkCnt);
		}
		else
		{
	//		TrPrintfL(TRUE, "REP_GET_BLOCK_USED idx=%d, cnt=%d", reply.blockIdx*8, reply.blkCnt*8);
	//		TrPrintfL(TRUE, "first  part: %d, %d", reply.blockIdx*8, (max-reply.blockIdx)*8);
	//		TrPrintfL(TRUE, "second part: %d, %d", min*8, (reply.blkCnt -(max-reply.blockIdx))*8);
			memcpy(reply.used,   				  Fpga.blockUsed+reply.blockIdx, 				  (max-reply.blockIdx));
			memcpy(reply.used+max-reply.blockIdx, Fpga.blockUsed+min, 				reply.blkCnt -(max-reply.blockIdx));
		}
		reply.hdr.msgLen = sizeof(reply)-sizeof(reply.used)+reply.blkCnt;
		sok_send(socket, &reply);
		return REPLY_OK;						
	}
	return Error(ERR_ABORT, 0, "Used Block %d out of range (%d..%d)!", msg->blockIdx, min, max);
}

//--- do_fpga_image -------------------------------------------------------
static int do_fpga_image		(RX_SOCKET socket, SFpgaImageCmd *msg)
{
	int				idx, cnt;
	SFpgaImageList	*list;

//	TrPrintfL(TRUE, "CMD_FPGA_IMAGE head=%d", msg->head);
	
	if (RX_HBConfig.dataBlkSize==0) return Error(ERR_ABORT, 0, "dataBlkSize=0");

	if (msg->head<MAX_HEADS_BOARD)
	{
		list = &Fpga.print->imageList[msg->head];
		idx = (list->inIdx+1)&0xff;
		if (idx==list->outIdx) return Error(ERR_CONT, 0, "Image List [%d] full", msg->head);
		memcpy(&sPageId[list->inIdx], &msg->id, sizeof(SPageId));
		memcpy(&list->image[list->inIdx], &msg->image, sizeof(SFpgaImage));
		cnt = (msg->image.widthBytes*msg->image.lengthPx+RX_HBConfig.dataBlkSize-1)/RX_HBConfig.dataBlkSize;

		TrPrintfL(TRUE, "CMD_FPGA_IMAGE image[%d.%d]: id=%d, page=%d, copy=%d, scan=%d, block0=%d..%d, blockCnt=%d, widthPx=%d, lengthPx=%d", msg->head, list->inIdx, msg->id.id,  msg->id.page, msg->id.copy, msg->id.scan, msg->image.blockNo, msg->image.blockNo+cnt, cnt, msg->image.widthPx, msg->image.lengthPx);

		list->inIdx = idx;

		if (TRUE)
		{
			SMsgHdr 		reply;
			reply.msgLen = sizeof(reply);
			reply.msgId  = REP_FPGA_IMAGE;
			sok_send(socket, &reply);
		}
	}
	return REPLY_OK;
}

//--- do_write_image -------------------------------------------------------
static int do_write_image(RX_SOCKET socket, SFpgaWriteBmpCmd *msg)
{
//	SIMU_write_image(msg->head, msg->imageNo);
	return REPLY_OK;
}

//--- do_head_board_cfg --------------------------------------------------
static int do_head_board_cfg 	(RX_SOCKET socket, SHeadBoardCfg *cfg)
{
	int i;

	TrPrintfL(TRUE, "do_head_board_cfg");

	sMainSocket = socket;
	memcpy(&RX_HBConfig, cfg, sizeof(RX_HBConfig));
	for (i=0; i<RX_HBConfig.headCnt; i++)
	{
		if (RX_HBConfig.head[i].blockNo0 % 8) Error(ERR_ABORT, 0, "Head[%d].blockNo0 must be a multiple of 8", i);
		if (RX_HBConfig.head[i].blockCnt % 8) Error(ERR_ABORT, 0, "Head[%d].blockCnt must be a multiple of 8", i);
	}
	fpga_set_config();
	memset(sPgCnt,  0, sizeof(sPgCnt));
//	memset(sPdCnt,  0, sizeof(sPdCnt));
	memset(sPageId, 0, sizeof(sPageId));
	sPdCnt = 0;

	//--- remove test bitmaps ---------
	{
		char path[100];
		sprintf(path, "%s*.bmp", PATH_TEMP);
		rx_remove_old_files(path, 0);
	}

	return REPLY_OK;
}

//--- do_simu_print ------------------------------------------------------
static int do_simu_print		(RX_SOCKET socket)
{
	simu_print(sPageId);
	return REPLY_OK;
}
