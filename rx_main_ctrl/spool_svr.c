// ****************************************************************************
//
//	spool_svr.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"
#include "enc_ctrl.h"
#include "network.h"
#include "label.h"
#include "spool_svr.h"

//--- Defines -----------------------------------------------------------------

#define MAX_SPOOLERS	5	

//--- Externals ---------------------------------------------------------------


//--- Statics -----------------------------------------------------------------
static HANDLE	_HSpoolServer;
static int		_Ready;
static int		_Auto;
static int		_BlkNo;
static int		_AwaitReply;
static int		_MsgSent, _MsgGot;
static int		_HeadBoardCnt;
static int		_SlideIsRight;
static SPrintFileCmd	_PrintFileMsg;



//--- Prototypes --------------------------------------------------------------
static int _handle_spool_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _handle_spool_open	(RX_SOCKET socket, const char *peerName);
static int _handle_spool_close	(RX_SOCKET socket, const char *peerName);

static int _do_spool_cfg_rep	(RX_SOCKET socket);
static int _do_print_file_rep	(RX_SOCKET socket, SPrintFileRep	*msg);
static int _do_print_file_evt	(RX_SOCKET socket, SPrintFileMsg	*msg);
static int _do_log_evt			(RX_SOCKET socket, SLogMsg			*msg);

// int send_head_configs(RX_SOCKET socket);


//--- spool_start ----------------------------------------------------------------
int	spool_start(void)
{
	int errNo;

	_Auto = FALSE;
	TrPrintfL(TRUE, "Spool started");
	errNo=sok_start_server(&_HSpoolServer, NULL, PORT_CTRL_MAIN, SOCK_STREAM, MAX_SPOOLERS, _handle_spool_msg, _handle_spool_open, _handle_spool_close);
	if (errNo)
	{
		char str[256];
		Error(ERR_CONT, 0, "Socket Error >>%s<<", err_system_error(errNo, str,  sizeof(str)));
	}
	return REPLY_OK;
}


//--- spool_auto -------------------------------------------------------------------------
void spool_auto(int enable)
{
	_Auto = enable;
}

//--- _handle_spool_msg ------------------------------------------------------------------
static int _handle_spool_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr	*phdr = (SMsgHdr*)msg;

	switch(phdr->msgId)
	{
	case CMD_REQ_SPOOL_CFG:	spool_set_config	(socket);							break;
	case REP_SET_SPOOL_CFG:	_do_spool_cfg_rep	(socket);							break;
	case REP_PRINT_FILE:	_do_print_file_rep	(socket, (SPrintFileRep*)	msg);	break;
	case REP_PRINT_ABORT:															break;
	case EVT_PRINT_FILE:	_do_print_file_evt	(socket, (SPrintFileMsg*)	msg);	break;

	case EVT_GET_EVT:		_do_log_evt			(socket, (SLogMsg*)			phdr);	break;

	case REP_RFS_SAVE_FILE_HDR:		label_rep_file_hdr();							break;
	case REP_RFS_SAVE_FILE_BLOCK:	label_rep_file_block();							break;
	default: Error(WARN, 0, "Got unknown MessageId=0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _handle_spool_open --------------------------------------------------------------------
static int _handle_spool_open(RX_SOCKET socket, const char *peerName)
{
    /*
	if (str_start(peerName, "127.0.0.1"))
	{
		// --- local spooler ---
		SNetworkItem item;
		memset(&item, 0, sizeof(item));
		item.deviceType = dev_spooler;
		strcpy(item.deviceTypeStr, DeviceStr[item.deviceType]);
		item.deviceNo   = 0;
		item.macAddr	= 2;
		strncpy(item.ipAddr, RX_CTRL_MAIN, sizeof(item.ipAddr)-1);
		net_register(&item);
	}
	*/
	spool_set_config(socket);
	return REPLY_OK;
}

//--- _handle_spool_close -------------------------------------------------------------------
static int _handle_spool_close(RX_SOCKET socket, const char *peerName)
{
    /*
	if (str_start(peerName, "127.0.0.1"))
	{
		SNetworkItem item;
		memset(&item, 0, sizeof(item));
		item.deviceType = dev_spooler;
		strcpy(item.deviceTypeStr, DeviceStr[item.deviceType]);
		item.deviceNo   = 0;
		item.macAddr	= 2;
		strncpy(item.ipAddr, RX_CTRL_MAIN, sizeof(item.ipAddr)-1);
		net_unregister(&item);
	}
	*/
	return REPLY_OK;
}

//--- _do_spool_cfg_rep ---------------------------------------------------------------------
static int _do_spool_cfg_rep(RX_SOCKET socket)
{
	return REPLY_OK;
}

//--- spool_set_config ---------------------------------------------------------------
// return: count of clients
int	spool_set_config(RX_SOCKET socket)
{
	int hb, cnt;
	char str[100];

	label_reset();
	sok_get_srv_socket_name(_HSpoolServer, NULL, str, NULL);
//	strncpy(RX_Spooler.dataRoot, RX_Config.ergosoft.root, sizeof(RX_Spooler.dataRoot));
	// data is always on MAIN
	if (strcmp(str, "127.0.0.1")) sprintf(RX_Spooler.dataRoot, "\\\\%s\\%s", str, PATH_RIPPED_DATA_DIR);
	else strcpy(RX_Spooler.dataRoot, PATH_RIPPED_DATA_DIR);
	cnt=sok_send_to_clients_2(_HSpoolServer, CMD_SET_SPOOL_CFG, sizeof(RX_Spooler), &RX_Spooler);

	//--- send head board configurations ------------------------
	_HeadBoardCnt=0;
	for (hb=0; hb<SIZEOF(RX_Config.headBoard); hb++)
	{
		if (RX_Config.headBoard[hb].present>=dev_on)
		{
			_HeadBoardCnt++;
			cnt=sok_send_to_clients_2(_HSpoolServer, CMD_HEAD_BOARD_CFG, sizeof(SHeadBoardCfg), &RX_Config.headBoard[hb]);
		}
	}

	//--- send bitmap split config (all info) --------------------------
	int color;
	for (color=0; color<SIZEOF(RX_Color); color++)
	{
		RX_Color[color].no=color;
		cnt=sok_send_to_clients_2(_HSpoolServer, CMD_COLOR_CFG, sizeof(RX_Color[color]), &RX_Color[color]);	
	}

	_Ready		= TRUE;
	_BlkNo		= 0;
	_AwaitReply = 0;
	_MsgSent = _MsgGot=0;
	RX_PrinterStatus.sentCnt=0;
//	pc_print_next();

	return cnt;
}

//--- spool_head_board_cnt --------------------------
int spool_head_board_cnt(void)
{
	return _HeadBoardCnt;
}

//--- spool_is_ready -------------------------------------------------------------------
int spool_is_ready(void)
{
	TrPrintfL(TRUE, "spool_ready: _Ready=%d, _AwaitReply=%d, _MsgSent=%d, _MsgGot=%d", _Ready, _AwaitReply, _MsgSent, _MsgGot);
	return (_Ready!=0) && (_AwaitReply==0);	
}

//--- spool_start_printing --------------------------------
void spool_start_printing(void)
{
	if (rx_process_running_cnt(FILENAME_SPOOLER_CTRL, NULL)==0)
	{
		#ifdef WIN32
			rx_process_start(PATH_BIN_WIN FILENAME_SPOOLER_CTRL".exe", NULL);
		#else
			rx_process_start(PATH_BIN_SPOOLER FILENAME_SPOOLER_CTRL, NULL);
		#endif
		rx_sleep(500);
		Error(WARN, 0, "rx_spooler_ctrl restarted");
	}

	_SlideIsRight = FALSE;
}

//--- spool_set_layout ---------------------------------------------------------------
int spool_set_layout(SLayoutDef *playout, char *dataPath)
{
	int		len, l;
	UCHAR *data;

	TrPrintfL(TRUE, "send spool_set_layout");
	sok_send_to_clients_2(_HSpoolServer, BEG_SET_LAYOUT, strlen(dataPath)+1, dataPath);
	for (len=sizeof(SLayoutDef), data=(UCHAR*)playout; len>0; )
	{
		if (len>2000) l=2000;
		else l=len;
		sok_send_to_clients_2(_HSpoolServer, ITM_SET_LAYOUT, l, data);
		data += l;
		len  -= l;
	}
	sok_send_to_clients_2(_HSpoolServer, END_SET_LAYOUT, 0, NULL);
	return REPLY_OK;
}

//--- spool_set_filedef --------------------------------------------------------
int spool_set_filedef(SFileDef  *pfileDef)
{
	int		len, l;
	UCHAR *data;

	TrPrintfL(TRUE, "send spool_set_filedef");
	sok_send_to_clients_2(_HSpoolServer, BEG_SET_FILEDEF, 0, NULL);
	for (len=sizeof(SFileDef), data=(UCHAR*)pfileDef; len>0; )
	{
		if (len>2000) l=2000;
		else l=len;
		sok_send_to_clients_2(_HSpoolServer, ITM_SET_FILEDEF, l, data);
		data += l;
		len  -= l;
	}
	sok_send_to_clients_2(_HSpoolServer, END_SET_FILEDEF, 0, NULL);
	return REPLY_OK;
}

//--- spool_set_counter --------------------------------------------------------
int spool_set_counter(SCounterDef  *pctrDef)
{
	TrPrintfL(TRUE, "send spool_set_counter");
	sok_send_to_clients_2(_HSpoolServer, CMD_SET_CTRDEF, sizeof(*pctrDef), pctrDef);
	return REPLY_OK;
}

//--- spool_send_test_data -----------------------------------------------------------
int spool_send_test_data(int headNo, char *str)
{
	SPrintTestDataMsg msg;

	msg.hdr.msgId  = CMD_PRINT_TEST_DATA;
	msg.hdr.msgLen = sizeof(msg); 
	msg.headNo = headNo;
	strncpy(msg.data, str, sizeof(msg.data));
	return sok_send_to_clients(_HSpoolServer, &msg);
}

//--- spool_print_file ---------------------------------------------------------------
int spool_print_file(SPageId *pid, const char *filename, INT32 offsetWidth, INT32 lengthPx, INT32 pgMode, INT32 pgDist, UINT8 lengthUnit, int variable, int scanMode, int clearBlockUsed)
{
	int				cnt;
//	if (_Ready<=0) 
//		 Error(WARN, 0, "Spooler not ready");
	_Ready--;
	
	_PrintFileMsg.hdr.msgLen	= sizeof(_PrintFileMsg);
	_PrintFileMsg.hdr.msgId		= CMD_PRINT_FILE;
	_PrintFileMsg.blkNo			= _BlkNo;
	_PrintFileMsg.variable		= variable;
	_PrintFileMsg.lengthUnit	= lengthUnit;
	_PrintFileMsg.mirror		= FALSE;
	_PrintFileMsg.clearBlockUsed= clearBlockUsed;
	strncpy(_PrintFileMsg.filename, filename, sizeof(_PrintFileMsg.filename));
	memcpy(&_PrintFileMsg.id, pid, sizeof(_PrintFileMsg.id));
	if (RX_PrinterStatus.testMode)
	{
		_PrintFileMsg.printMode     = PM_TEST;
		if (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_ENCODER)
		{
			if (RX_Config.printer.type==printer_test_table) _PrintFileMsg.printMode = PM_TEST_SINGLE_COLOR;
		}
			
		_PrintFileMsg.offsetWidth	= 0;
		_PrintFileMsg.lengthPx		= lengthPx;
		_PrintFileMsg.gapPx			= 0;	// unused
	}
	else if (rx_def_is_scanning(RX_Config.printer.type))
	{
		_PrintFileMsg.printMode     = PM_SCANNING;
		_PrintFileMsg.offsetWidth	= offsetWidth;
		_PrintFileMsg.lengthPx		= lengthPx;
		_PrintFileMsg.gapPx			= (UINT32)(pgDist*1.200/25.4);
	}
	else
	{
		_PrintFileMsg.printMode     = PM_SINGLE_PASS;
		_PrintFileMsg.offsetWidth	= microns_to_px(offsetWidth, DPI_X);
		_PrintFileMsg.lengthPx		= 0;	// unused
		_PrintFileMsg.gapPx			= 0;	// unused
	}
	switch (scanMode)
	{
	case PQ_SCAN_BIDIR:	_PrintFileMsg.mirror = IMAGE_BIDIR | _SlideIsRight;	_SlideIsRight=!_SlideIsRight; break;
	case PQ_SCAN_RTL:	_PrintFileMsg.mirror = TRUE;  _SlideIsRight=TRUE;	break;
	default:			_PrintFileMsg.mirror = FALSE; _SlideIsRight=FALSE;	break;
	}
		
	TrPrintfL(TRUE, "send spool_print_file >>%s<<", filename);
	cnt=sok_send_to_clients(_HSpoolServer, &_PrintFileMsg);
	if (cnt) RX_PrinterStatus.sentCnt++;// = _HeadBoardCnt;
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	sok_send_to_clients_2(_HSpoolServer, CMD_PING, 0, NULL);	// needed in linux to send the command without timeout!
	_AwaitReply += cnt;
	_MsgSent+=cnt;
	TrPrintfL(TRUE, "****** sent spool_print_file to %d spoolers: _AwaitReply=%d, _MsgSent=%d", cnt, _AwaitReply, _MsgSent);
//	Error(LOG, 0, "spool_print_file Copy %d", pid->copy);
	return REPLY_OK;
}

/*
//--- spool_print_empty ----------------------------
int spool_print_empty(void)
{
	int				cnt;

//	if (_Ready<=0) 
//		 Error(WARN, 0, "Spooler not ready");
	_Ready--;
		
//	memset(&_PrintFileMsg.id, 0, sizeof(_PrintFileMsg.id));

	Error(LOG, 0, "Bugfix: Send Extra Image");
	_PrintFileMsg.id.scan = 0xffffffff;
	cnt=sok_send_to_clients(_HSpoolServer, &_PrintFileMsg);
	// do not count!
	if (cnt) RX_PrinterStatus.sentCnt += 0;// _HeadBoardCnt;
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	sok_send_to_clients_2(_HSpoolServer, CMD_PING, 0, NULL);	// needed in linux to send the command without timeout!
	_AwaitReply += cnt;
	_MsgSent+=cnt;
	TrPrintfL(TRUE, "****** sent spool_print_file to %d spoolers: _AwaitReply=%d, _MsgSent=%d", cnt, _AwaitReply, _MsgSent);
//	Error(LOG, 0, "spool_print_file Copy %d", pid->copy);
	return REPLY_OK;
}
*/

//--- spool_abort_printing --------------------------------------
int spool_abort_printing(void)
{	
	sok_send_to_clients_2(_HSpoolServer, CMD_PRINT_ABORT, 0, NULL);
	return REPLY_OK;
}

//--- _do_print_file_rep ------------------------------------------------------
static int _do_print_file_rep(RX_SOCKET socket, SPrintFileRep *msg)
{
	int size;

	--_AwaitReply;
	_MsgGot++;
//	if (msg->bufReady)
	{
		TrPrintfL(TRUE, "****** rep_print_file id=%d, page=%d, copy=%d, scan=%d, width=%d, length=%d, bufReady=%d, _AwaitReply=%d, _MsgGot=%d", msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->widthPx, msg->lengthPx, msg->bufReady, _AwaitReply, _MsgGot);
		size = ((RX_Spooler.headWidthPx+RX_Spooler.headOverlapPx)*msg->bitsPerPixel+7)/8;
		size *= msg->lengthPx;
		size = (size+RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
	
		_BlkNo = (_BlkNo+size) % RX_Spooler.dataBlkCntHead;
		
		_Ready = msg->bufReady;
		pc_print_next();
	}
	/*
	else
	{
		TrPrintfL(TRUE, "\n**** rep_print_file id=%d, page=%d, copy=%d, scan=%d, width=%d, length=%d, bufReady=%d *****\n", msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->widthPx, msg->lengthPx, msg->bufReady);
		
		_Ready = msg->bufReady;
		if (!msg->bufReady)
			_Ready=_Ready;		
	}
	*/

	return REPLY_OK;
}

//--- _do_print_file_evt ------------------------------------------------------
static int _do_print_file_evt	(RX_SOCKET socket, SPrintFileMsg	*msg)
{
	TrPrintfL(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: EVENT %d",	msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt);
	SPrintQueueItem *pitem;
	switch (msg->evt)
	{
	case DATA_RIPPING:	TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: RIPPING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_SCREENING:TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SCREENING, copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_LOADING:	TrPrintfL(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: LOADING,   copy=%d, scan=%d, bufReady=%d >>%s<<",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, msg->txt);
						pq_loading(msg->spoolerNo, &msg->id, msg->txt);
						break;

	case DATA_SENDING:	TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SENDING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); 
						pq_sending(msg->spoolerNo, &msg->id); 
						break;

	case DATA_SENT:		TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SENT,      copy=%d, scan=%d, bufReady=%d,  _AwaitReply=%d, _MsgGot=%d",		msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, _AwaitReply, _MsgGot);
						pitem = pq_sent(&msg->id);
						pc_sent(&msg->id);
						if (RX_PrinterStatus.testMode)	enc_set_pg(&RX_TestImage, &msg->id);
						else							enc_set_pg(pitem, &msg->id);
						if (_Auto)  ctrl_print_page(&msg->id);
						break;
	default:			TrPrintf(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: UNKNOWN EVENT %d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt); break;
	}
	_Ready = msg->bufReady;
	pc_print_next();
	return REPLY_OK;
}

//--- _do_log_evt -----------------------------------------------------
static int _do_log_evt(RX_SOCKET socket, SLogMsg *msg)
{
	EDevice device;
	int no;
	
	ctrl_get_device(socket, &device, &no);
	SlaveError(device, no, &msg->log);
	return REPLY_OK;
}

//--- spool_send_msg ----------------------------------------------
int spool_send_msg(void *msg)
{
	return sok_send_to_clients(_HSpoolServer, msg);
}

//--- spool_send_msg_2 ----------------------------------------
int spool_send_msg_2(UINT32 cmd, int dataSize, void *data)
{
	return sok_send_to_clients_2(_HSpoolServer, cmd, dataSize, data);								
}

