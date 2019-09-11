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
#include "args.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "plc_ctrl.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"
#include "enc_ctrl.h"
#include "network.h"
#include "label.h"
#include "spool_svr.h"

//--- Defines -----------------------------------------------------------------

#define MAX_SPOOLERS	8	
#define MAX_PAGES		128

//--- Externals ---------------------------------------------------------------

typedef struct
{
	int			used;
	int			err;
	RX_SOCKET	socket;
	int			loadNo;
	int			sendNo;
	int			sentNo;
} SSpoolerInfo;

//--- Statics -----------------------------------------------------------------
static HANDLE	_HSpoolServer;
static int		_Ready;
static int		_Auto;
static int		_BlkNo;
static int		_SpoolerCnt;
static int		_MsgSent, _MsgGot;
static int		_HeadBoardCnt;
static int		_SlideIsRight;
static int		_Pass;
static int		_DelayPauseTimer=0;
static SSpoolerInfo	 _Spooler[MAX_SPOOLERS];
static SPageId		_Id[MAX_PAGES];

//--- Prototypes --------------------------------------------------------------
static int _handle_spool_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static int _handle_spool_connected	(RX_SOCKET socket, const char *peerName);
static int _handle_spool_deconnected(RX_SOCKET socket, const char *peerName);

static int _do_spool_cfg_rep	(RX_SOCKET socket);
static int _do_print_file_rep	(RX_SOCKET socket, int spoolerNo, SPrintFileRep	*msg);
static int _do_print_file_evt	(RX_SOCKET socket, SPrintFileMsg	*msg);
static void _do_print_done_evt	(RX_SOCKET socket, SPrintDoneMsg	*msg);
static int _do_log_evt			(RX_SOCKET socket, SLogMsg			*msg);
static int _do_pause_printing	(RX_SOCKET socket);
static int _do_start_printing	(RX_SOCKET socket);

// int send_head_configs(RX_SOCKET socket);


//--- spool_start ----------------------------------------------------------------
int	spool_start(void)
{
	int errNo, i;
	char addr[32];

	_Auto = FALSE;
	TrPrintfL(TRUE, "Spool started");
	memset(_Spooler, 0, sizeof(_Spooler));
	memset(_Id, 0, sizeof(_Id));
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


//--- spool_tick ---------------------------------------
void spool_tick(void)
{
	if (_DelayPauseTimer && rx_get_ticks()>_DelayPauseTimer)
	{
		_DelayPauseTimer = 0;
		Error(LOG, 0, "PAUSE to load file");
		plc_pause_printing();
		pc_print_next();			
	}
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
	int spoolerNo;
	
	for (spoolerNo=0; _Spooler[spoolerNo].socket!=socket; spoolerNo++);
	
//	TrPrintfL(TRUE, "received Spooler[%d].MsgId=0x%08x", spoolerNo, phdr->msgId);
	
	switch(phdr->msgId)
	{
	case CMD_REQ_SPOOL_CFG:	spool_set_config	(socket);							break;
	case REP_SET_SPOOL_CFG:	_do_spool_cfg_rep	(socket);							break;
	case REP_PRINT_FILE:	_do_print_file_rep	(socket, spoolerNo, (SPrintFileRep*)	msg);	break;
	case REP_PRINT_ABORT:															break;
	case EVT_PRINT_FILE:	_do_print_file_evt	(socket, (SPrintFileMsg*)	msg);	break;
	case EVT_PRINT_DONE:	_do_print_done_evt	(socket, (SPrintDoneMsg*)	msg);	break;
		
	case EVT_GET_EVT:		_do_log_evt			(socket, (SLogMsg*)			phdr);	break;

	case REP_RFS_SAVE_FILE_HDR:		label_rep_file_hdr();							break;
	case REP_RFS_SAVE_FILE_BLOCK:	label_rep_file_block();							break;
		
	case CMD_PAUSE_PRINTING:	_do_pause_printing(socket);							break;
	case CMD_START_PRINTING:	_do_start_printing(socket);							break;
		
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
	if (no<SIZEOF(_Spooler))
	{
		_Spooler[no].err	= FALSE;
		_Spooler[no].socket = socket;
	}
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


//--- _do_pause_printing -------------------------------------------------------------------
static int _do_pause_printing	(RX_SOCKET socket)
{
	if (arg_simuPLC)
	{
		Error(LOG, 0, "PAUSE to load file");
		plc_pause_printing();
		pc_print_next();			
	}
	else _DelayPauseTimer = rx_get_ticks()+1000;
	return REPLY_OK;
}

//--- _do_start_printing ------------------------------------------------------------------
static int _do_start_printing	(RX_SOCKET socket)
{
	_DelayPauseTimer = 0;
	TrPrintfL(TRUE, "CONTINUE after file loaded");
	Error(LOG, 0, "CONTINUE after file loaded");
	plc_start_printing();
	pc_print_next();
	return REPLY_OK;
}

//--- spool_set_config ---------------------------------------------------------------
// return: count of clients
int	spool_set_config(RX_SOCKET socket)
{
	int hb, cnt, no;
	char str[100];
	int color;
	
	_DelayPauseTimer = 0;

	for (no=0; no<SIZEOF(_Spooler); no++)
	{
		_Spooler[no].used=FALSE;
		_Spooler[no].loadNo=0;
		_Spooler[no].sendNo=0;
		_Spooler[no].sentNo=0;
	}
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
		cnt=spool_send_msg_2(CMD_COLOR_CFG,		sizeof(RX_Color[color]),		&RX_Color[color],		 FALSE);
		RX_DisabledJets[color].color = color;
		cnt=spool_send_msg_2(CMD_DISABLED_JETS,	sizeof(RX_DisabledJets[color]), &RX_DisabledJets[color], FALSE);
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
//	TrPrintfL(TRUE, "spool_ready: _Ready=%d, _MsgSent=%d, _MsgGot=%d", _Ready, _MsgSent, _MsgGot);
	return (_Ready!=0) && (_MsgSent==_MsgGot);
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
	_Pass		  = 0;
}

//--- spool_start_sending --------------------------------
void spool_start_sending(void)
{
	spool_send_msg_2(CMD_START_PRINTING, 0, NULL, TRUE);
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
int spool_print_file(SPageId *pid, const char *filename, INT32 offsetWidth, INT32 lengthPx, SPrintQueueItem *pitem, int clearBlockUsed)
{
//	if (_Ready<=0) 
//		 Error(WARN, 0, "Spooler not ready");

	SPrintFileCmd msg;
	
	if (arg_simuHeads) Error(LOG, 0, "Printing ID=%d, page=%d, copy=%d", pid->id, pid->page, pid->copy);

	_Ready--;
	
	memset(&msg, 0, sizeof(msg));
	msg.hdr.msgLen		= sizeof(msg);
	msg.hdr.msgId		= CMD_PRINT_FILE;
	msg.blkNo			= _BlkNo;
	msg.variable		= pitem->variable;
	msg.lengthUnit		= pitem->lengthUnit;
	msg.flags			= 0;
	msg.clearBlockUsed	= clearBlockUsed;
	msg.wakeup			= pitem->wakeup;
	strncpy(msg.filename, filename, sizeof(msg.filename));
	memcpy(&msg.id, pid, sizeof(msg.id));
	memcpy(&_Id[RX_PrinterStatus.sentCnt%MAX_PAGES], pid, sizeof(msg.id));
	if (RX_PrinterStatus.testMode)
	{
		msg.printMode     = PM_TEST;
		if (RX_Config.printer.type==printer_test_table && (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_JET_NUMBERS) || RX_TestImage.testImage==PQ_TEST_ENCODER)
		{
			msg.printMode = PM_TEST_SINGLE_COLOR;
		}
		else if (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_JET_NUMBERS) 
		{
			msg.printMode = PM_TEST_JETS;
		}
			
		msg.offsetWidth	= 0;
		msg.lengthPx	= lengthPx;
		msg.gapPx		= 0;	// unused
	}
	else if (rx_def_is_scanning(RX_Config.printer.type))
	{
		if(pitem->srcPages>1)
		{
			msg.printMode=PM_SCAN_MULTI_PAGE;
			if(pid->page==pitem->start.page)						  msg.flags |= FLAG_SMP_FIRST_PAGE;
			if(pid->page==pitem->lastPage && pid->scan==pitem->scans) msg.flags |= FLAG_SMP_LAST_PAGE;
			msg.smp_bufSize = pitem->scansStart;
		}
		else msg.printMode = PM_SCANNING;

		msg.offsetWidth	= offsetWidth;
		msg.lengthPx	= lengthPx;
		msg.gapPx		= (UINT32)(pitem->printGoDist*1.200/25.4);
	}
	else
	{
		msg.printMode   = PM_SINGLE_PASS;
		msg.offsetWidth	= microns_to_px(offsetWidth, DPI_X);
		msg.lengthPx	= 0;	// unused
		msg.gapPx		= 0;	// unused
	}
	switch (pitem->scanMode)
	{
	case PQ_SCAN_BIDIR:	msg.flags |= (FLAG_BIDIR | _SlideIsRight); _SlideIsRight=!_SlideIsRight;	break;
	case PQ_SCAN_RTL:	msg.flags |= FLAG_MIRROR;  _SlideIsRight=TRUE;								break;
	default:			_SlideIsRight=FALSE;	break;
	}

	if (pitem->virtualPasses) 		
	{
		msg.virtualPasses = pitem->passes;
		msg.virtualPass   = _Pass;
		_Pass			  = (_Pass+1) % pitem->passes;
	}

//	TrPrintfL(TRUE, "send spool_print_file >>%s<<", filename);
	int cnt;
	cnt=spool_send_msg(&msg);
	if (cnt) RX_PrinterStatus.sentCnt++;// = _HeadBoardCnt;
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	Error(LOG, 0, "Load File[%d] page=%d, scan=%d", RX_PrinterStatus.sentCnt, pid->page, pid->scan);
	
//	spool_send_msg_2(CMD_PING, 0, NULL, TRUE);	// needed in linux to send the command without timeout!
	_MsgSent+=cnt;
	TrPrintfL(TRUE, "****** sent spool_print_file (id=%d, page=%d, copy=%d, scan=%d) to %d spoolers: _MsgSent=%d", pid->id, pid->page, pid->copy, pid->scan, cnt);
//	Error(LOG, 0, "spool_print_file Copy %d", pid->copy);
	return REPLY_OK;
}

//--- spool_get_id ---------------------------------
SPageId *spool_get_id(int no)
{
	return &_Id[no%MAX_PAGES];
}

//--- spool_abort_printing --------------------------------------
int spool_abort_printing(void)
{	
	spool_send_msg_2(CMD_PRINT_ABORT, 0, NULL, FALSE);
	return REPLY_OK;
}

//--- _do_print_file_rep ------------------------------------------------------
static int _do_print_file_rep(RX_SOCKET socket, int spoolerNo, SPrintFileRep *msg)
{
	int size;

	_MsgGot++;
//	if (msg->bufReady)
	{
		TrPrintfL(TRUE, "****** Spooler[%d]:rep_print_file id=%d, page=%d, copy=%d, scan=%d, width=%d, length=%d, bufReady=%d, _MsgSent=%d, _MsgGot=%d", spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->widthPx, msg->lengthPx, msg->bufReady, _MsgSent, _MsgGot);
		size = ((RX_Spooler.headWidthPx+RX_Spooler.headOverlapPx)*msg->bitsPerPixel+7)/8;
		size *= msg->lengthPx;
		size = (size+RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
	
		if (msg->clearBlockUsed)
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
//	TrPrintfL(TRUE, "Documment (id=%d, page=%d, scan=%d, copy=%d): EVENT %d",	msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt);
	SPrintQueueItem *pitem;
	switch (msg->evt)
	{
	case DATA_RIPPING:	TrPrintfL(TRUE, "SPOOLER %d: RIPPING (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_SCREENING:TrPrintfL(TRUE, "SPOOLER %d: SCREENING (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); break;
	case DATA_LOADING:	TrPrintfL(TRUE, "SPOOLER %d: LOADING #%d: (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d >>%s<<",	msg->spoolerNo, ++_Spooler[msg->spoolerNo].loadNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, msg->txt);
						pq_loading(msg->spoolerNo, &msg->id, msg->txt);
						break;

	case DATA_SENDING:	TrPrintf(TRUE, "SPOOLER %d: SENDING #%d: (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d",	msg->spoolerNo, ++_Spooler[msg->spoolerNo].sendNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady); 
						pq_sending(msg->spoolerNo, &msg->id); 
						break;

	case DATA_SENT:		TrPrintf(TRUE, "SPOOLER %d: SENT #%d: (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d,  _MsgSent=%d, _MsgGot=%d",	msg->spoolerNo, ++_Spooler[msg->spoolerNo].sentNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, _MsgSent, _MsgGot);
						pitem = pq_sent(&msg->id);							
						if(RX_PrinterStatus.testMode || pitem==NULL)
						{
							RX_PrinterStatus.transferredCnt++;
							pc_sent(&msg->id);
							enc_set_pg(&RX_TestImage, &msg->id);
						}
						else if (_SpoolerCnt==0 || (pitem->scansSent%_SpoolerCnt)==0)
						{
							TrPrintf(TRUE, "*** SENT #%d *** (id=%d, page=%d, scan=%d, copy=%d)", _Spooler[msg->spoolerNo].sentNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan);							
							RX_PrinterStatus.transferredCnt++;
							pc_sent(&msg->id);
							enc_set_pg(pitem, &msg->id);
						}							
						if (_Auto)  ctrl_print_page(&msg->id);																					
						break;
	/*
	case DATA_PRINT_DONE:	// simu_write from spooler
						for (int i=0; i<_HeadBoardCnt; i++)
						{
							pc_printed(&msg->id, i);
						}
						break;
	*/
	default:			TrPrintf(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: UNKNOWN EVENT %d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt); break;
	}
	_Ready = msg->bufReady;
	pc_print_next();
	return REPLY_OK;
}

//--- _do_print_done_evt ---------------------
static void _do_print_done_evt	(RX_SOCKET socket, SPrintDoneMsg *msg)
{
	for (int i=0; i<_HeadBoardCnt; i++)
		pc_print_done(msg->boardNo, msg);
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
			if (_Spooler[i].socket!=INVALID_SOCKET && sok_send_2(&_Spooler[i].socket,cmd,dataSize,data)==REPLY_OK) cnt++;
			else if(errmsg) 
			{
				if (!_Spooler[i].err) ErrorEx(dev_spooler, i, ERR_ABORT, 0, "not connected");
				_Spooler[i].err = TRUE;
			}
		}
	}
	return cnt;
}

