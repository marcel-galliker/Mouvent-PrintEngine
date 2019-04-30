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

#define MAX_SPOOLERS	8	

//--- Externals ---------------------------------------------------------------

typedef struct
{
	int			used;
	RX_SOCKET	socket;
} SSpoolerInfo;

//--- Statics -----------------------------------------------------------------
static HANDLE	_HSpoolServer;
static int		_Ready;
static int		_Auto;
static int		_BlkNo;
static int		_AwaitReply;
static int		_SpoolerCnt;
static int		_MsgSent, _MsgGot;
static int		_HeadBoardCnt;
static int		_SlideIsRight;
static SPrintFileCmd _PrintFileMsg;
static SSpoolerInfo	 _Spooler[MAX_SPOOLERS];


//--- Prototypes --------------------------------------------------------------
static int _handle_spool_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _handle_spool_connected	(RX_SOCKET socket, const char *peerName);
static int _handle_spool_deconnected(RX_SOCKET socket, const char *peerName);

static int _do_spool_cfg_rep	(RX_SOCKET socket);
static int _do_print_file_rep	(RX_SOCKET socket, SPrintFileRep	*msg);
static int _do_print_file_evt	(RX_SOCKET socket, SPrintFileMsg	*msg);
static int _do_log_evt			(RX_SOCKET socket, SLogMsg			*msg);

// int send_head_configs(RX_SOCKET socket);


//--- spool_start ----------------------------------------------------------------
int	spool_start(void)
{
	int errNo, i;
	char addr[32];

	_Auto = FALSE;
	TrPrintfL(TRUE, "Spool started");
	memset(_Spooler, 0, sizeof(_Spooler));
	for(i=0; i<SIZEOF(_Spooler); i++) 
	{	
		net_device_to_ipaddr(dev_spooler, i, addr, sizeof(addr));
		_Spooler[i].socket = INVALID_SOCKET;
	}
	
	errNo=sok_start_server(&_HSpoolServer, NULL, PORT_CTRL_MAIN, SOCK_STREAM, MAX_SPOOLERS, _handle_spool_msg, _handle_spool_connected, _handle_spool_deconnected);
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

//--- _handle_spool_connected --------------------------------------------------------------------
static int _handle_spool_connected(RX_SOCKET socket, const char *peerName)
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
	EDevice device;
	int no;
	net_ipaddr_to_device(peerName, &device, &no);
	if (no<SIZEOF(_Spooler)) _Spooler[no].socket = socket;
	spool_set_config(socket);
	return REPLY_OK;
}

//--- _handle_spool_deconnected -------------------------------------------------------------------
static int _handle_spool_deconnected(RX_SOCKET socket, const char *peerName)
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
	EDevice device;
	int no;
	net_ipaddr_to_device(peerName, &device, &no);
	if (no<SIZEOF(_Spooler)) _Spooler[no].socket = INVALID_SOCKET;
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
	int hb, cnt, no;
	char str[100];
	int color;

	for (no=0; no<SIZEOF(_Spooler); no++) _Spooler[no].used=FALSE;
	_SpoolerCnt=0;
	//--- send bitmap split config (all info) --------------------------
	for (color=0; color<SIZEOF(RX_Color); color++)
	{
		RX_Color[color].no=color;
		no = RX_Color[color].spoolerNo;
		if (no<SIZEOF(_Spooler))
		{
			if (!_Spooler[no].used) _SpoolerCnt++;
			_Spooler[no].used = TRUE;
		}
	}

	label_reset();
	sok_get_srv_socket_name(_HSpoolServer, NULL, str, NULL);
//	strncpy(RX_Spooler.dataRoot, RX_Config.ergosoft.root, sizeof(RX_Spooler.dataRoot));
	// data is always on MAIN
	if (strcmp(str, "127.0.0.1")) sprintf(RX_Spooler.dataRoot, "\\\\%s\\%s", str, PATH_RIPPED_DATA_DIR);
	else strcpy(RX_Spooler.dataRoot, PATH_RIPPED_DATA_DIR);
	cnt=spool_send_msg_2(CMD_SET_SPOOL_CFG, sizeof(RX_Spooler), &RX_Spooler, FALSE);

	//--- send bitmap split config (all info) --------------------------
	if (RX_Config.printer.type==printer_DP803) Error(LOG, 0, "Send CMD_COLOR_CFG only to used spooler");
	for (color=0; color<SIZEOF(RX_Color); color++)
	{
		cnt=spool_send_msg_2(CMD_COLOR_CFG, sizeof(RX_Color[color]), &RX_Color[color], FALSE);	
	}

	//--- send head board configurations ------------------------
	_HeadBoardCnt=0;
	if (RX_Config.printer.type==printer_DP803) Error(LOG, 0, "Send CMD_HEAD_BOARD_CFG only to used spooler"); // RX_Config.headBoard[hb].spoolerNo
	for (hb=0; hb<SIZEOF(RX_Config.headBoard); hb++)
	{
		if (RX_Config.headBoard[hb].present>=dev_on)
		{
			_HeadBoardCnt++;
			cnt=spool_send_msg_2(CMD_HEAD_BOARD_CFG, sizeof(SHeadBoardCfg), &RX_Config.headBoard[hb], FALSE);
		}
	}
	
	/*
	//--- send bitmap split config (all info) --------------------------
	for (color=0; color<SIZEOF(RX_Color); color++)
	{
		RX_Color[color].no=color;
		cnt=spool_send_msg_2(CMD_COLOR_CFG, sizeof(RX_Color[color]), &RX_Color[color], FALSE);	
	}
	*/
	
	_Ready		= TRUE;
	_BlkNo		= 0;
	_AwaitReply = 0;
	_MsgSent	= _MsgGot=0;
	RX_PrinterStatus.sentCnt=0;
	RX_PrinterStatus.transferredCnt=0;
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
	spool_send_msg_2(BEG_SET_LAYOUT, strlen(dataPath)+1, dataPath, TRUE);
	for (len=sizeof(SLayoutDef), data=(UCHAR*)playout; len>0; )
	{
		if (len>2000) l=2000;
		else l=len;
		spool_send_msg_2(ITM_SET_LAYOUT, l, data, TRUE);
		data += l;
		len  -= l;
	}
	spool_send_msg_2(END_SET_LAYOUT, 0, NULL, TRUE);
	return REPLY_OK;
}

//--- spool_set_filedef --------------------------------------------------------
int spool_set_filedef(SFileDef  *pfileDef)
{
	int		len, l;
	UCHAR *data;

	TrPrintfL(TRUE, "send spool_set_filedef");
	spool_send_msg_2(BEG_SET_FILEDEF, 0, NULL, TRUE);
	for (len=sizeof(SFileDef), data=(UCHAR*)pfileDef; len>0; )
	{
		if (len>2000) l=2000;
		else l=len;
		spool_send_msg_2(ITM_SET_FILEDEF, l, data, TRUE);
		data += l;
		len  -= l;
	}
	spool_send_msg_2(END_SET_FILEDEF, 0, NULL, TRUE);
	return REPLY_OK;
}

//--- spool_set_counter --------------------------------------------------------
int spool_set_counter(SCounterDef  *pctrDef)
{
	TrPrintfL(TRUE, "send spool_set_counter");
	spool_send_msg_2(CMD_SET_CTRDEF, sizeof(*pctrDef), pctrDef, TRUE);
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
	return spool_send_msg(&msg);
}

//--- spool_print_file ---------------------------------------------------------------
int spool_print_file(SPageId *pid, const char *filename, INT32 offsetWidth, INT32 lengthPx, INT32 pgMode, INT32 pgDist, UINT8 lengthUnit, int variable, int scanMode, int clearBlockUsed)
{
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
		if (RX_Config.printer.type==printer_test_table && RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_ENCODER)
		{
			 _PrintFileMsg.printMode = PM_TEST_SINGLE_COLOR;
		}
		else if (RX_TestImage.testImage==PQ_TEST_JETS) 
		{
			_PrintFileMsg.printMode = PM_TEST_JETS;
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
	int cnt;
	cnt=spool_send_msg(&_PrintFileMsg);
	if (cnt) RX_PrinterStatus.sentCnt++;// = _HeadBoardCnt;
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	spool_send_msg_2(CMD_PING, 0, NULL, TRUE);	// needed in linux to send the command without timeout!
	_AwaitReply += cnt;
	_MsgSent+=cnt;
	TrPrintfL(TRUE, "****** sent spool_print_file to %d spoolers: _AwaitReply=%d, _MsgSent=%d", _AwaitReply, _MsgSent);
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
	cnt=spool_send_msg(&_PrintFileMsg);
	// do not count!
	if (cnt) RX_PrinterStatus.sentCnt += 0;// _HeadBoardCnt;
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	spool_send_msg_2(CMD_PING, 0, NULL, TRUE);	// needed in linux to send the command without timeout!
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
	spool_send_msg_2(CMD_PRINT_ABORT, 0, NULL, FALSE);
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
	case DATA_RIPPING:	TrPrintfL(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: RIPPING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_SCREENING:TrPrintfL(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SCREENING, copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_LOADING:	TrPrintfL(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: LOADING,   copy=%d, scan=%d, bufReady=%d >>%s<<",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, msg->txt);
						pq_loading(msg->spoolerNo, &msg->id, msg->txt);
						break;

	case DATA_SENDING:	TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SENDING,   copy=%d, scan=%d, bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); 
						pq_sending(msg->spoolerNo, &msg->id); 
						break;

	case DATA_SENT:		TrPrintf(TRUE, "SPOOLER %d: Documment ID=%d  page=%d: SENT,      copy=%d, scan=%d, bufReady=%d,  _AwaitReply=%d, _MsgGot=%d",		msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, _AwaitReply, _MsgGot);
						pitem = pq_sent(&msg->id);							
						if(RX_PrinterStatus.testMode)
						{
							RX_PrinterStatus.transferredCnt++;
							pc_sent(&msg->id);
							enc_set_pg(&RX_TestImage, &msg->id);
						}
						else
						{
							if (_SpoolerCnt==0 || (pitem->scansSent%_SpoolerCnt)==0)
							{
								RX_PrinterStatus.transferredCnt++;
								pc_sent(&msg->id);
								enc_set_pg(pitem, &msg->id);
							}							
						}			
						if (_Auto)  ctrl_print_page(&msg->id);																					
						break;
		
	case DATA_PRINT_DONE:	// simu_write from spooler
						for (int i=0; i<_HeadBoardCnt; i++)
						{
							pc_printed(&msg->id, i);
						}
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
	int i, cnt;
	for (i=0, cnt=0; i<SIZEOF(_Spooler); i++)
	{
		if(_Spooler[i].used)
		{
			if (_Spooler[i].socket!=INVALID_SOCKET && sok_send(&_Spooler[i].socket,msg)==REPLY_OK) cnt++; 
			else ErrorEx(dev_spooler, i, ERR_ABORT, 0, "not connected");
		}
	}
	return cnt;
}

//--- spool_send_msg_2 ----------------------------------------
int spool_send_msg_2(UINT32 cmd, int dataSize, void *data, int errmsg)
{
	int i, cnt;
	for (i=0, cnt=0; i<SIZEOF(_Spooler); i++)
	{
		if(_Spooler[i].used)
		{
			if (_Spooler[i].socket!=INVALID_SOCKET && sok_send_2(&_Spooler[i].socket,INADDR_ANY,cmd,dataSize,data)==REPLY_OK) cnt++;
			else if(errmsg) 
				ErrorEx(dev_spooler, i, ERR_ABORT, 0, "not connected");
		}
	}
	return cnt;
}

