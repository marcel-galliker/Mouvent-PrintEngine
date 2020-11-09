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

typedef struct
{
	SPageId id;
	int		blkNo;
	int		blkCnt;
	int		offsetWidth;
} SLoadedFiles;

//--- Statics -----------------------------------------------------------------
static HANDLE	_HSpoolServer;
static int		_Ready;
static int		_ErrorSpooler=FALSE;
static int		_Auto;
static int		_BlkNo;
static int		_SpoolerCnt;
static int		_MsgSent, _MsgGot;
static UINT32	_HeadBoardUsedFlags;
static int		_Pass;
static int		_DelayPauseTimer=0;
static SSpoolerInfo		_Spooler[MAX_SPOOLERS];
static SPageId			_Id[MAX_PAGES];
static int			_ActId;
static SLoadedFiles		_LoadedFiles[MAX_PAGES];

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
static void _start_spooler_ctrl(int no);

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
	memset(_LoadedFiles, 0, sizeof(_LoadedFiles));
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
		plc_spooler_pause_printing();
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
	
	for (spoolerNo=0; spoolerNo<MAX_SPOOLERS; spoolerNo++)
	{
		if (_Spooler[spoolerNo].socket==socket)
		{
		    //	TrPrintfL(TRUE, "received Spooler[%d].MsgId=0x%08x", spoolerNo, phdr->msgId);
    
	        switch(phdr->msgId)
	        {
	        case CMD_REQ_SPOOL_CFG:	spool_set_config	(socket, ctrl_headResetCnt());				break;
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
	}
	return REPLY_ERROR;
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
	spool_set_config(socket, ctrl_headResetCnt());
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
	if (no<SIZEOF(_Spooler)) 
	{
		ErrorEx(dev_spooler, no, ERR_ABORT, 0, "Connection lost");
		_Spooler[no].socket = INVALID_SOCKET;
	//	_start_spooler_ctrl(no);
	}
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
    if (RX_PrinterStatus.printState == ps_printing)
    {
		if (arg_simuPLC)
		{
			Error(LOG, 0, "PAUSE to load file");
			plc_spooler_pause_printing();
			pc_print_next();			
		}
		else _DelayPauseTimer = rx_get_ticks()+1000;
    }
    return REPLY_OK;
}

//--- _do_start_printing ------------------------------------------------------------------
static int _do_start_printing	(RX_SOCKET socket)
{
	_DelayPauseTimer = 0;
	TrPrintfL(TRUE, "CONTINUE after file loaded");
	Error(LOG, 0, "CONTINUE after file loaded");
	plc_spooler_start_printing();
	pc_print_next();
	return REPLY_OK;
}

//--- spool_set_config ---------------------------------------------------------------
// return: count of clients
int	spool_set_config(RX_SOCKET socket, UINT32 resetCnt)
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
	RX_Spooler.resetCnt = resetCnt;
	cnt=spool_send_msg_2(CMD_SET_SPOOL_CFG, sizeof(RX_Spooler), &RX_Spooler, FALSE);		

	//--- send bitmap split config (all info) --------------------------
	if (RX_Config.printer.type==printer_DP803) Error(LOG, 0, "Send CMD_COLOR_CFG only to used spooler");

	for (color=0; color<SIZEOF(RX_Color); color++)
	{
		cnt=spool_send_msg_2(CMD_COLOR_CFG,		sizeof(RX_Color[color]),		&RX_Color[color],		 FALSE);
		for (no=0; no<RX_Config.headsPerColor; no++)
		{
			if (RX_Color[color].lastLine)
			{
				int head  = color*RX_Config.headsPerColor+no;
				{
					SDisabledJetsMsg msg;
					msg.hdr.msgId  = CMD_SET_DISABLED_JETS;
					msg.hdr.msgLen = sizeof(msg);
					msg.head  = head;
					memcpy(msg.disabledJets, RX_HBStatus[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].eeprom_mvt.disabledJets, sizeof(msg.disabledJets));
					cnt=spool_send_msg(&msg);
				}
				{
					SDensityValuesMsg msg;
					msg.hdr.msgId  = CMD_SET_DENSITY_VAL;
					msg.hdr.msgLen = sizeof(msg);
					msg.head  = head;
					memcpy(msg.value, RX_HBStatus[head/MAX_HEADS_BOARD].head[head%MAX_HEADS_BOARD].eeprom_mvt.densityValue, sizeof(msg.value));
					if (FALSE)
					{
						char str[128];
						int len = sprintf(str, "scr_set_values[%d.%d]: ", head/MAX_HEADS_BOARD, head%MAX_HEADS_BOARD);
						for (int i=0; i<MAX_DENSITY_VALUES; i++) len += sprintf(&str[len], "%d ", msg.value[i]);
						TrPrintfL(TRUE, str);
						Error(LOG, 0, str);
					}
					cnt=spool_send_msg(&msg);
				}
			}
		}
	}

	//--- send head board configurations ------------------------
	_HeadBoardUsedFlags=0;
	if (RX_Config.printer.type==printer_DP803) Error(LOG, 0, "Send CMD_HEAD_BOARD_CFG only to used spooler"); // RX_Config.headBoard[hb].spoolerNo
	for (hb=0; hb<SIZEOF(RX_Config.headBoard); hb++)
	{
		if (RX_Config.headBoard[hb].present>=dev_on)
		{
			_HeadBoardUsedFlags|=(1<<hb);
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

//--- spool_head_board_used_flags --------------------------
UINT32 spool_head_board_used_flags(void)
{
	return _HeadBoardUsedFlags;
}

//--- spool_is_ready -------------------------------------------------------------------
int spool_is_ready(void)
{	
//	TrPrintfL(TRUE, "spool_ready: _Ready=%d, _MsgSent=%d, _MsgGot=%d", _Ready, _MsgSent, _MsgGot);
	return (_Ready!=0) && (_MsgSent==_MsgGot);
}

//--- _start_spooler_ctrl ----------------------
static void _start_spooler_ctrl(int no)
{
	if (no==0)
	{
		#ifdef WIN32
			rx_process_start(PATH_BIN_WIN FILENAME_SPOOLER_CTRL".exe", NULL);
		#else
			rx_process_start(PATH_BIN_SPOOLER FILENAME_SPOOLER_CTRL, NULL);
		#endif
//		rx_sleep(500);
		Error(WARN, 0, "rx_spooler_ctrl[%d] restarted", no);
	}
	else Error(ERR_ABORT, 0, "Starting spooler no!=0 not implemented");
}

//--- spool_start_printing --------------------------------
void spool_start_printing(void)
{	
	//--- DP803: Check that all spoolers are running  -----
	if (RX_Config.printer.type==printer_DP803)
	{
		Error(LOG, 0, "Check or restart spoolers");
	}

//	#ifndef DEBUG
	//--- check local spooler is running ------------------
	if (rx_process_running_cnt(FILENAME_SPOOLER_CTRL, NULL)==0)
	{
		_start_spooler_ctrl(0);
		/*
		#ifdef WIN32
			rx_process_start(PATH_BIN_WIN FILENAME_SPOOLER_CTRL".exe", NULL);
		#else
			rx_process_start(PATH_BIN_SPOOLER FILENAME_SPOOLER_CTRL, NULL);
		#endif
		rx_sleep(500);
		Error(WARN, 0, "rx_spooler_ctrl restarted");
		*/
	}
//	#endif
	memset(_LoadedFiles, 0, sizeof(_LoadedFiles));

	_ActId		  = 0;
	_Pass		  = 0;
}

//--- spool_start_sending --------------------------------
void spool_start_sending(UINT32 resetCnt)
{
	spool_send_msg_2(CMD_START_PRINTING, sizeof(resetCnt), &resetCnt, TRUE);
}

//--- spool_set_layout ---------------------------------------------------------------
int spool_set_layout(SLayoutDef *playout, char *dataPath)
{
	int		len, l;
	UCHAR *data;

	TrPrintfL(TRUE, "send spool_set_layout");
	spool_send_msg_2(BEG_SET_LAYOUT, (int)strlen(dataPath)+1, dataPath, TRUE);
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

//--- spool_load_file ---------------------------------------------------------------------
int spool_load_file(SPageId *pid, char *path)
{
	SLoadFileCmd msg;
	int cnt;

	msg.hdr.msgLen		= sizeof(msg);
	msg.hdr.msgId		= CMD_LOAD_FILE;
	memcpy(&msg.id, pid, sizeof(msg.id));
	strncpy(msg.filepath, path, sizeof(msg.filepath));
	cnt=spool_send_msg(&msg);

	return REPLY_OK;
}

//--- spool_print_file ---------------------------------------------------------------
int spool_print_file(SPageId *pid, const char *filename, INT32 offsetWidth, INT32 lengthPx, SPrintQueueItem *pitem, int clearBlockUsed)
{
	static int	_mirror = FALSE;
//	if (_Ready<=0) 
//		 Error(WARN, 0, "Spooler not ready");

	SPrintFileCmd msg;

	if (arg_simuHeads) Error(LOG, 0, "Printing ID=%d, page=%d, copy=%d, scan=%d", pid->id, pid->page, pid->copy, pid->scan);
	
	_Ready--;
	
	memset(&msg, 0, sizeof(msg));
	msg.hdr.msgLen		= sizeof(msg);
	msg.hdr.msgId		= CMD_PRINT_FILE;
	msg.blkNo			= _BlkNo;
	msg.variable		= pitem->variable;
	msg.lengthUnit		= pitem->lengthUnit;
	msg.flags			= 0;
	msg.clearBlockUsed	= clearBlockUsed;
	msg.wakeup			  = pitem->wakeup;
	msg.penetrationPasses = pitem->penetrationPasses;
	
	strncpy(msg.filename, filename, sizeof(msg.filename));
	strncpy(msg.dots,     pitem->dots, sizeof(msg.dots));
	memcpy(&msg.id, pid, sizeof(msg.id));
	memcpy(&_Id[RX_PrinterStatus.sentCnt%MAX_PAGES], pid, sizeof(msg.id));

	if (rx_def_is_lb(RX_Config.printer.type) && RX_Config.printer.type!=printer_DP803 && !RX_PrinterStatus.testMode)
	{
		int i;
		SPageId *p;
		int ow=microns_to_px(offsetWidth, DPI_X);
		for (i=0; i<SIZEOF(_LoadedFiles); i++)
		{
			p=&_LoadedFiles[i].id;
			if ((p->id==pid->id) && (p->page==pid->page) && (p->scan==pid->scan) && ow==_LoadedFiles[i].offsetWidth) // copy changes! 
			{
				msg.flags |= FLAG_SAME;
				msg.blkNo = _LoadedFiles[i].blkNo;
				break;				
			}	
		}
		if(pitem->srcPages>1 && pid->copy<pitem->copies) msg.clearBlockUsed=FALSE;
	//	if (arg_simuHeads) Error(LOG, 0, "same=%d, clearBlockUsed=%d", msg.flags&FLAG_SAME, msg.clearBlockUsed);
	//	Error(LOG, 0, "spool_print_file (id=%d, page=%d, copy=%d): same=%d, clearBlockUsed=%d", pid->id, pid->page, pid->copy, msg.flags&FLAG_SAME, msg.clearBlockUsed);
	}
	
	if (RX_PrinterStatus.testMode)
	{
		msg.printMode     = PM_TEST;
		if (RX_Config.printer.type==printer_test_table && (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_JET_NUMBERS  || RX_TestImage.testImage==PQ_TEST_DENSITY))
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
		/*
		else 
		{
			static int _lastid=0;
			if (_lastid!=pid->id) 
			{ // plc_ctrl::ctrl_send_head_cfg()
				_lastid		= pid->id;
				_BlkNo		= 0;
				msg.blkNo	= _BlkNo;				
			}
			msg.printMode = PM_SCANNING;
		}
		*/
		
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

	if (pid->id!=_ActId) 
	{
		_ActId = pid->id;
		_mirror=FALSE;
	}
						
	switch (pitem->scanMode)
	{
	case PQ_SCAN_BIDIR: msg.flags |= (FLAG_BIDIR | _mirror); _mirror=!_mirror;	break;
	case PQ_SCAN_RTL:	msg.flags |= FLAG_MIRROR;								break;
	default:																	break;
	}

	if (pitem->virtualPasses)
	{
		static int _lastId=0;
		if (pid->id!=_lastId) _Pass=0;
		_lastId = pid->id;
		msg.virtualPasses = pitem->passes;
		msg.virtualPass   = _Pass;
		_Pass			  = (_Pass+1) % pitem->passes;
	}
	else 
	{
		_Pass=0;
		msg.virtualPass=0;
	}
	
//	TrPrintfL(TRUE, "send spool_print_file >>%s<<", filename);
	int cnt;
	cnt=spool_send_msg(&msg);
	if (cnt)
	{
		pq_preflight(&msg.id);
		RX_PrinterStatus.sentCnt++;
	}
	else     Error(ERR_CONT, 0, "No Spoolers connected");
//	Error(LOG, 0, "Load File[%d] page=%d, scan=%d", RX_PrinterStatus.sentCnt, pid->page, pid->scan);
	
//	spool_send_msg_2(CMD_PING, 0, NULL, TRUE);	// needed in linux to send the command without timeout!
	_MsgSent+=cnt;
	TrPrintfL(TRUE, "****** sent spool_print_file (id=%d, page=%d, copy=%d, scan=%d) to %d spoolers: _MsgSent=%d", pid->id, pid->page, pid->copy, pid->scan, cnt, _MsgSent);
//	Error(LOG, 0, "spool_print_file Copy %d", pid->copy);
	return REPLY_OK;
}

//--- spool_file_printed -----------------------------------
void spool_file_printed(SPageId *pid)
{
	int i;
	SPageId *p;
	for (i=0; i<SIZEOF(_LoadedFiles); i++)
	{
		p=&_LoadedFiles[i].id;
		if ((p->id==pid->id) && (p->page==pid->page) && (p->scan==pid->scan))
		{
			memset(&_LoadedFiles[i], 0, sizeof(_LoadedFiles[i]));
			break;				
		}	
	}		
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
	_ErrorSpooler=FALSE;
	return REPLY_OK;
}

//--- _do_print_file_rep ------------------------------------------------------
static int _do_print_file_rep(RX_SOCKET socket, int spoolerNo, SPrintFileRep *msg)
{
	int i;
	_MsgGot++;
//	if (msg->bufReady)
	{
		TrPrintfL(TRUE, "****** Spooler[%d]:rep_print_file id=%d, page=%d, copy=%d, scan=%d, bufReady=%d, same=%d, _MsgSent=%d, _MsgGot=%d", spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->bufReady, msg->same, _MsgSent, _MsgGot);
			
		if (msg->clearBlockUsed)
		{
			for (i=0; i<SIZEOF(_LoadedFiles); i++)
			{
				SPageId *pid = &_LoadedFiles[i].id;
				if (pid->id==msg->id.id && pid->page==msg->id.page && pid->scan==msg->id.scan && _LoadedFiles[i].offsetWidth==msg->offsetWidth)
				{
					memset(&_LoadedFiles[i], 0, sizeof(_LoadedFiles[i]));
				}
			}
		}

		if (!msg->same)
		{
			for (i=0; i<SIZEOF(_LoadedFiles); i++)
			{
				if (_LoadedFiles[i].blkCnt==0)
				{
					memcpy(&_LoadedFiles[i].id, &msg->id, sizeof(_LoadedFiles[i].id));
					_LoadedFiles[i].offsetWidth = msg->offsetWidth;
					_LoadedFiles[i].blkNo		= _BlkNo;
					_LoadedFiles[i].blkCnt		= msg->blkCnt;
					break;
				}
			}
			_BlkNo = (_BlkNo+msg->blkCnt) % RX_Spooler.dataBlkCntHead;				
		}
		
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

	case DATA_SENT:		TrPrintf(TRUE, "SPOOLER %d: SENT #%d: (id=%d, page=%d, scan=%d, copy=%d), bufReady=%d,  _MsgSent=%d, _MsgGot=%d",	msg->spoolerNo, ++_Spooler[msg->spoolerNo].sentNo, msg->id.id, msg->id.page, msg->id.scan, msg->id.copy, msg->bufReady, _MsgSent, _MsgGot);
						pitem = pq_sent(&msg->id);							
						if(RX_PrinterStatus.testMode || pitem==NULL)
						{
							RX_PrinterStatus.transferredCnt++;
							pc_sent(&msg->id);
							enc_set_pg(&RX_TestImage, &msg->id);
						}
						else if (_SpoolerCnt==0 || (pitem->scansSent%_SpoolerCnt)==0)
						{
							TrPrintf(TRUE, "*** SENT #%d *** (id=%d, page=%d, scan=%d, copy=%d)", _Spooler[msg->spoolerNo].sentNo, msg->id.id, msg->id.page, msg->id.scan, msg->id.copy);							
							RX_PrinterStatus.transferredCnt++;
							pc_sent(&msg->id);
							enc_set_pg(pitem, &msg->id);
						}							
						if (_Auto)  ctrl_print_page(&msg->id);																					
						break;

		default:			TrPrintf(TRUE, "Documment ID=%d  page=%d , copy=%d, scan=%d: UNKNOWN EVENT %d",	msg->spoolerNo, msg->id.id, msg->id.page, msg->id.copy, msg->id.scan, msg->evt); break;
	}
	_Ready = msg->bufReady;
	pc_print_next();
	return REPLY_OK;
}

//--- _do_print_done_evt ---------------------
static void _do_print_done_evt	(RX_SOCKET socket, SPrintDoneMsg *msg)
{
	RX_PrinterStatus.printGoCnt++;
	for (int i=0; i<8*sizeof(_HeadBoardUsedFlags); i++)
		if (_HeadBoardUsedFlags & (1<<i))
			pc_print_done(i, msg);
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
	int i, cnt, err=FALSE;
	for (i=0, cnt=0; i<SIZEOF(_Spooler); i++)
	{
		if(_Spooler[i].used)
		{
			if (_Spooler[i].socket!=INVALID_SOCKET && sok_send(&_Spooler[i].socket,msg)==REPLY_OK) cnt++;
		}
	}
	if (!cnt) 
	{
		if (!_ErrorSpooler) ErrorEx(dev_spooler, 0, ERR_ABORT, 0, "not connected");
		_ErrorSpooler = TRUE;
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

