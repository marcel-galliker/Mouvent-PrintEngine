// ****************************************************************************
//
//	ctrl_client.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_counter.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mem.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "args.h"
#include "data.h"
#include "data_client.h"
#include "bmp.h"
#include "jet_correction.h"
#include "rx_rip_lib.h"
#include "spool_rip.h"
#include "head_client.h"
#include "ctrl_client.h"

//--- Defines -----------------------------------------------------------------
// #define MAX_ETH_PORT	2


//--- Externals ---------------------------------------------------------------

//--- Structures --------------------------------------------------------------

//--- statics -----------------------------------------------------------------
static int				_ThreadRunning;
static RX_SOCKET		_RxCtrlSocket=INVALID_SOCKET;


static void				*_print_file_thread(void *par);
static HANDLE			_PrintFile_Sem;
static RX_SOCKET		_PrintFile_Socket;
static SPrintFileCmd	_PrintFile_Msg;

static int				_Running;
static int				_Abort;
static int				_ReadyToSend;
static int				_Paused;
static SFSDirEntry		_DirEntry;
static FILE				*_File=NULL;
static char				_LastFilename[MAX_PATH];
static int				_LastPage;
static int				_LastWakeup;
static UINT16			_SMP_Flags;
#define					BUFFER_CNT 2
static int				_BufferNo;
static UINT64			_BufferSize[BUFFER_CNT];
static BYTE*			_Buffer[BUFFER_CNT][MAX_COLORS]; // [MAX_HEADS_COLOR];	// buffered in case of same image

int					_MsgGot, _MsgSent, _MsgGot0;
int					_MsgId=0;

//--- prototypes --------------------------------------------------------------

static void *_main_ctrl_thread(void *par);
static int _handle_main_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);

static int _do_spool_cfg	(RX_SOCKET socket, SSpoolerCfg	  *cfg);
static int _do_color_cfg	(RX_SOCKET socket, SColorSplitCfg *cfg);
static int _do_disabled_jets(RX_SOCKET socket, SDisabledJets  *jets);
static int _do_print_file	(RX_SOCKET socket, SPrintFileCmd  *msg);
static void _do_start_sending(void);
static int _do_print_abort	(RX_SOCKET socket);
static int _do_print_test_data(RX_SOCKET socket, SPrintTestDataMsg *msg);

static int _do_save_file_hdr	(RX_SOCKET socket, SFSDirEntry   *msg);
static int _do_save_file_block	(RX_SOCKET socket, SDataBlockMsg *msg);

// static int do_print_file_color(SColorSplitCfg *pcfg, SBmpInfo *info, SPrintFileCmd *msg);
static int load_bitmaps	(const char *filepath, SBmpInfo *info);

//--- ctrl_start---------------------------------------------------------------
int ctrl_start(const char *ipAddrMain)
{
	_ThreadRunning  = TRUE;
	_Running		= FALSE;
	_BufferNo       = BUFFER_CNT-1;
	memset(_BufferSize, 0, sizeof(_BufferSize));
	memset(_Buffer, 0, sizeof(_Buffer));
	memset(_LastFilename, 0, sizeof(_LastFilename));
	_LastPage=0;
	_LastWakeup = 0;
	_SMP_Flags=0;

	rx_mem_init(512*1024*1024);
	if (!_PrintFile_Sem) _PrintFile_Sem = rx_sem_create();

	rx_thread_start(_main_ctrl_thread, (void*)ipAddrMain, 0, "_main_ctrl_thread");
	rx_thread_start(_print_file_thread, NULL,             0, "_print_file_thread");
	hc_start();

	return REPLY_OK;
}

//--- ctrl_end ---------------------------------------------------------------
int ctrl_end(void)
{
	hc_end();
	_ThreadRunning = FALSE;
	return REPLY_OK;
}

//--- ctrl_send ---------------------------------------------------------------
void ctrl_send(void *msg)
{
	sok_send(&_RxCtrlSocket, msg);
}

//--- ctrl_pause_printing -----------------------------------------------
void ctrl_pause_printing(void)
{
	if (!_Paused) sok_send_2(&_RxCtrlSocket, CMD_PAUSE_PRINTING, 0, NULL);			
	_Paused = TRUE;
}

//--- ctrl_start_printing -------------------------------------------------
void ctrl_start_printing(void)
{
	if (_Paused) sok_send_2(&_RxCtrlSocket, CMD_START_PRINTING, 0, NULL);
	_Paused = FALSE;
}

//--- ctrl_send_load_progress ---------------------------------------------------------
void ctrl_send_load_progress(SPageId *id, const char *colorSN, int progress)
{
	SPrintFileMsg msg;
	
	TrPrintfL(TRUE, "Loading >>%s<<, %d%%", colorSN, progress);
	memset(&msg, 0, sizeof(msg));
	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId  = EVT_PRINT_FILE;
	msg.evt        = DATA_LOADING;
	msg.spoolerNo  = RX_SpoolerNo;
	msg.bufReady   = data_ready();
	memcpy(&msg.id, id, sizeof(SPageId));
	if( id->page < 2) snprintf(msg.txt, sizeof(msg.txt) - 1, "%s %d%%", colorSN, progress);		
	else			  snprintf(msg.txt, sizeof(msg.txt)-1, "page %d %s %d%%", id->page, colorSN, progress);
	sok_send(&_RxCtrlSocket, &msg);
}

//---  _main_ctrl_thread ------------------------------------------------------------
static void *_main_ctrl_thread(void *par)
{
	int errNo, lastErrNo=0;
	char *ipAddrMain = (char*)par;

	while (_ThreadRunning)
	{
		errNo=sok_open_client(&_RxCtrlSocket, ipAddrMain, PORT_CTRL_MAIN, SOCK_STREAM);
		if (errNo!=REPLY_OK)
		{
			if (errNo!=lastErrNo)
			{
				char str[128];
				TrPrintfL(TRUE, "%s: Socket Error >>%s<<", ipAddrMain, err_system_error(errNo, str, sizeof(str)));	
			}
		}
		else
		{
			TrPrintfL(TRUE, "CONNECTED to rx_main_ctrl");
			err_set_client(_RxCtrlSocket);
//			sok_send_2(_RxCtrlSocket, CMD_REQ_SPOOL_CFG, 0, NULL);
			sok_receiver(NULL, &_RxCtrlSocket, _handle_main_ctrl_msg, NULL);
			_RxCtrlSocket = INVALID_SOCKET;
			TrPrintfL(TRUE, "DISCONNECTED from rx_main_ctrl");
			_do_print_abort(INVALID_SOCKET);
			if (!arg_debug) rx_exit(0);
		}
		lastErrNo = errNo;
		rx_sleep(1000);
	}
	return NULL;
}

//---  _print_file_thread ------------------------------------------------------------
static void *_print_file_thread(void *par)
{
	while (_ThreadRunning)
	{
		if (rx_sem_wait(_PrintFile_Sem, 1000)==REPLY_OK)
		{
			_do_print_file(_PrintFile_Socket, &_PrintFile_Msg);
		}				
	}
	return NULL;
}

//--- _handle_main_ctrl_msg --------------------------------------------------------
static int _handle_main_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*)msg;
	static UINT32 _UnknownMsgId=0;

	_MsgId = phdr->msgId;
	switch(phdr->msgId)
	{
	case CMD_PING:					break;

	case CMD_SET_SPOOL_CFG:			_do_spool_cfg		(socket, (SSpoolerCfg*)		&phdr[1]);	break;
	case CMD_HEAD_BOARD_CFG:		hc_head_board_cfg	(socket, (SHeadBoardCfg*)	&phdr[1]);	break;
	case CMD_COLOR_CFG:				_do_color_cfg		(socket, (SColorSplitCfg*)	&phdr[1]);	break;
	case CMD_DISABLED_JETS:			_do_disabled_jets	(socket, (SDisabledJets*)	&phdr[1]);	break;
	case CMD_PRINT_FILE:			_MsgGot0++;
									_PrintFile_Socket = socket;
									memcpy(&_PrintFile_Msg, msg, sizeof(_PrintFile_Msg));
									rx_sem_post(_PrintFile_Sem);
									break;

	case CMD_START_PRINTING:		_do_start_sending	();										break;
	case CMD_PRINT_ABORT:			_do_print_abort		(socket);								break;
	
	case BEG_SET_LAYOUT:			sr_set_layout_start (socket, (char*)&phdr[1]);				break;
	case ITM_SET_LAYOUT:			sr_set_layout_blk   (socket, phdr);							break;
	case END_SET_LAYOUT:			sr_set_layout_end   (socket, phdr);							break;
	
	case BEG_SET_FILEDEF:			sr_set_filedef_start(socket, phdr);							break;
	case ITM_SET_FILEDEF:			sr_set_filedef_blk  (socket, phdr);							break;
	case END_SET_FILEDEF:			sr_set_filedef_end  (socket, phdr);							break;

	case CMD_SET_CTRDEF:			ctr_set_def	((SCounterDef*)		&phdr[1]);					break;
		
	case CMD_PRINT_DATA:			sr_data_record		((SPrintDataMsg*) msg);					break;

	case CMD_PRINT_TEST_DATA:		_do_print_test_data(socket, (SPrintTestDataMsg*)msg);	    break;

	case CMD_RFS_SAVE_FILE_HDR:		_do_save_file_hdr	(socket, (SFSDirEntry*)  msg);			break;
	case CMD_RFS_SAVE_FILE_BLOCK:	_do_save_file_block(socket, (SDataBlockMsg*)msg);			break;
//	case CMD_FONTS_UPDATED:			rip_add_fonts(PATH_FONTS);									break;
	case CMD_FONTS_UPDATED:			rip_add_fonts((char*)&phdr[1]);								break;

	default:					
		if (phdr->msgId != _UnknownMsgId)
		{
			Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
			_UnknownMsgId = phdr->msgId;
		}
	}
	_MsgId=0;
	return REPLY_OK;
}

//--- _do_print_test_data ------------------------------------------------------------
static int _do_print_test_data(RX_SOCKET socket, SPrintTestDataMsg *msg)
{
	if (msg->headNo>=0 && msg->headNo<SIZEOF(RX_TestData))
		strncpy(RX_TestData[msg->headNo], msg->data, sizeof(RX_TestData[msg->headNo]));
	return REPLY_OK;
}

//--- _do_spool_cfg --------------------------------------------------------------
static int _do_spool_cfg(RX_SOCKET socket, SSpoolerCfg *pmsg)
{	
	TrPrintfL(TRUE, "got REP_GET_SPOOL_CFG");
	memcpy(&RX_Spooler, pmsg, sizeof(RX_Spooler));
	sr_mnt_drive(RX_Spooler.dataRoot);
	jc_init();
	data_init(socket, RX_Spooler.colorCnt*RX_Spooler.headsPerColor);
	{
		int i;
		for (i=0; i<SIZEOF(_Buffer); i++)
			data_clear(_Buffer[i]);		
	}
	_Running	 = FALSE;
	_Abort		 = FALSE;
	_ReadyToSend = FALSE;
	_Paused		 = FALSE;
	_SMP_Flags	 = 0;
	_MsgGot = _MsgSent = _MsgGot0 = 0;
	sok_send_2(&socket, REP_SET_SPOOL_CFG, 0, NULL);
	sr_reset();
	if (rx_def_is_test(RX_Spooler.printerType) || !rx_def_is_scanning(RX_Spooler.printerType)) memset(_LastFilename, 0, sizeof(_LastFilename));

	// data_free(_Buffer);
	return REPLY_OK;
}

//--- _do_color_cfg ---------------------------------------------------------------------
static int _do_color_cfg		(RX_SOCKET socket, SColorSplitCfg* cfg)
{
	TrPrintfL(TRUE, "_do_color_cfg[%d], >>%s<<, firstLine=%d, lastLine=%d", cfg->no, cfg->color.name, cfg->firstLine, cfg->lastLine);

	if (cfg->no<SIZEOF(RX_Color))	
	{
		memcpy(&RX_Color[cfg->no], cfg, sizeof(RX_Color[cfg->no]));
		if (cfg->spoolerNo!=RX_SpoolerNo)
		{
			RX_Color[cfg->no].firstLine = RX_Color[cfg->no].lastLine = 0;
		}
		if (cfg->color.name[0])
		{
			if (RX_Spooler.printerType==printer_DP803)	RX_ColorNameInit(cfg->inkSupplyNo, cfg->rectoVerso, cfg->color.fileName, cfg->color.colorCode);
			else										RX_ColorNameInit(cfg->inkSupplyNo, rv_undef, cfg->color.fileName, cfg->color.colorCode);				
		}
	}
	else Error(ERR_CONT, 0, "CMD_COLOR_CFG no=%d out of range", cfg->no);
	return REPLY_OK;
}

//--- _do_disabled_jets ---------------------------------------------------------------------
static int _do_disabled_jets(RX_SOCKET socket, SDisabledJets *jets)
{
	jc_set_disabled_jets(jets);
	return REPLY_OK;
}

//--- _do_print_file ----------------------------------------------------------------------
static int _do_print_file(RX_SOCKET socket, SPrintFileCmd  *pdata)
{
	SPrintFileCmd   msg;
	int				ret=REPLY_OK;
	int				same;
	static SPrintFileRep	reply;
	static int				widthPx, lengthPx;
	static UCHAR			bitsPerPixel;
	static UINT8			multiCopy;
	static char				path[MAX_PATH];
		
	_MsgGot++;
	memcpy(&msg, pdata, sizeof(msg)); // local copy as original can be overwritten by the communication task!

	_Running = TRUE;
	hc_start_printing();
	
	same = (!strcmp(msg.filename, _LastFilename) && msg.id.page==_LastPage && msg.wakeup==_LastWakeup);
	if (RX_Spooler.printerType==printer_LB702_UV && msg.printMode==PM_SINGLE_PASS) same = ((msg.flags&FLAG_SAME)!=0);

	TrPrintfL(TRUE, "_do_print_file[%d] >>%s<<  id=%d, page=%d, copy=%d, scan=%d, same=%d, blkNo=%d", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan ,same, msg.blkNo);
	
//	if(msg.smp_flags & SMP_LAST_PAGE) Error(LOG, 0, "_do_print_file[%d] >>%s<< id=%d, page=%d, copy=%d, scan=%d, same=%d, LAST PAGE", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, same); 
//	else                                Error(LOG, 0, "_do_print_file[%d] >>%s<< id=%d, page=%d, copy=%d, scan=%d, same=%d", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, same); 
	
//	Error(LOG, 0, "Spooler Copy=%d, Offset=%d", msg.id.copy, msg.offsetWidth);
	
	if (!same)
	{
	//	Error(LOG, 0, "load file >>%s<< id=%d, page=%d, copy=%d", msg.filename, msg.id.id, msg.id.page, msg.id.copy);
		_BufferNo = (_BufferNo+1)%BUFFER_CNT;

//		if(msg.printMode==PM_SCANNING || msg.printMode==PM_SINGLE_PASS) // test
		if(msg.printMode==PM_SCANNING) // correct!
			data_set_wakeuplen(WAKEUP_BAR_LEN*(RX_Spooler.colorCnt+1), msg.wakeup);
		else			 
			data_set_wakeuplen(0, FALSE);
		memset(_LastFilename, 0, sizeof(_LastFilename));
	//	sr_mnt_path(msg.filename, path);
		data_cache(&msg.id, msg.filename, path, RX_Color, SIZEOF(RX_Color));
		
		//--- allocate memory ---------------------------------------------------------------------
		if ( msg.variable)// && !*path)
		{
			sr_get_label_size(&widthPx, &lengthPx, &bitsPerPixel);
		}
		else
		{
			ret = data_get_size(path, msg.id.page, msg.gapPx, &widthPx, &lengthPx, &bitsPerPixel, &multiCopy);
			if (ret==REPLY_NOT_FOUND) 
				Error(ERR_STOP, 0, "Could not load file >>%s<<", msg.filename);
		}
		if (msg.printMode==PM_SCAN_MULTI_PAGE) multiCopy=0;
		if (ret==REPLY_OK)
		{
			if (msg.smp_bufSize) data_clear(_Buffer[_BufferNo]);
			ret = data_malloc (msg.printMode, widthPx, lengthPx, bitsPerPixel, RX_Color, SIZEOF(RX_Color), multiCopy, &_BufferSize[_BufferNo], _Buffer[_BufferNo]);			
		}
		if (_Abort) 
			return REPLY_ERROR;
		if (ret==REPLY_ERROR)
		{
			if (multiCopy>1) return Error(ERR_STOP, 0,     "Could not allocate memory file >>%s<<, page=%d, copy=%d, scan=%d (Make Image width a multilpe of 4)", path, msg.id.page, msg.id.copy, msg.id.scan);
			else			 return Error(ERR_STOP, 0,     "Could not allocate memory file >>%s<<, page=%d, copy=%d, scan=%d", path, msg.id.page, msg.id.copy, msg.id.scan);
		}
	}

	//--- send feedback ---------------------------------------------------------------------
	{
		int linelenBt;
		reply.hdr.msgId  = REP_PRINT_FILE;
		reply.hdr.msgLen = sizeof(reply);
		reply.same   = same;
		linelenBt = ((HEAD_WIDTH_SAMBA+HEAD_OVERLAP_SAMBA)*bitsPerPixel+8)/8;	// 7=max pixel shifting
		linelenBt = (linelenBt+31) & ~31;// align to 256 Bits (32 Bytes)
		reply.blkCnt = (linelenBt *lengthPx +RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
		memcpy(&reply.id, &msg.id, sizeof(reply.id));
		reply.bufReady   = data_ready();
		ret=sok_send(&socket, &reply);		
	}

	if (ret==0) _MsgSent++;
	else
		TrPrintfL(TRUE, "sok_send PRINT-REPLY=%d", ret);

	TrPrintfL(TRUE, "REP_PRINT_FILE _MsgGot0=%d, _MsgGot=%d, _MsgSent=%d, widthPx=%d, lengthPx=%d, same=%d", _MsgGot0, _MsgGot, _MsgSent, widthPx, lengthPx, same);
//	if (_PrintFileDone_Sem) 	rx_sem_post(_PrintFileDone_Sem);
	 
	if (ret==REPLY_OK || !*path)
	{ 
		SPrintFileMsg	evt;
		if (msg.flags & FLAG_SMP_LAST_PAGE) _SMP_Flags |= FLAG_SMP_LAST_PAGE;
		
		if (RX_Spooler.printerType==printer_LB702_UV && msg.flags&FLAG_SAME)
		{
			data_same(&msg.id);				
		}
		else
		{
		//	Error(LOG, 0, "Spooler Copy=%d, Offset=%d: Load Data", msg.id.copy, msg.offsetWidth);
	//		if (data_load(&msg.id, path, msg.offsetWidth, msg.lengthPx, msg.gapPx, msg.blkNo, msg.printMode, msg.variable, msg.flags, msg.clearBlockUsed, same, _SMP_Flags | (msg.flags & FLAH_SMP_FIRST_PAGE), msg.smp_bufSize, _Buffer[_BufferNo])!=REPLY_OK && !_Abort)
			if (data_load(&msg.id, path, msg.offsetWidth, msg.lengthPx, multiCopy, msg.gapPx, msg.blkNo, reply.blkCnt, msg.printMode, msg.variable, msg.virtualPasses , msg.virtualPass, msg.flags, msg.clearBlockUsed, same, msg.smp_bufSize, _Buffer[_BufferNo])!=REPLY_OK && !_Abort)
				return Error(ERR_STOP, 0, "Could not load file >>%s<<", str_start_cut(msg.filename, PATH_RIPPED_DATA));					
		}
		
		//--- send spool state -------------------------------------------------------------
 		evt.hdr.msgId   = EVT_PRINT_FILE;
		evt.hdr.msgLen  = sizeof(evt);
		memcpy(&evt.id, &msg.id, sizeof(evt.id));
		evt.spoolerNo   = RX_SpoolerNo;
		evt.evt			= DATA_SENDING;
		evt.bufReady    = reply.bufReady;
		sok_send(&socket, &evt);
		
		TrPrintfL(TRUE, "REPLY EVT_PRINT_FILE, bufReady=%d, _ReadyToSend=%d", evt.bufReady, _ReadyToSend);
		if (hc_in_simu()) _ReadyToSend=TRUE;
		if (_ReadyToSend) hc_send_next();
		memcpy(&_LastFilename, &msg.filename, sizeof(_LastFilename));
		_LastPage = msg.id.page;
		_LastWakeup = msg.wakeup;
	}
	return REPLY_OK;
}

//--- _do_start_sending ----------------------------
static void _do_start_sending(void)
{
	_ReadyToSend = TRUE;
	hc_send_next();
}
	
//--- _do_print_abort ----------------------------------------------------------------------
static int _do_print_abort(RX_SOCKET socket)
{
	TrPrintfL(TRUE, "_do_print_abort");
	_Abort=TRUE;
	data_abort();
	hc_abort_printing();
	sok_send_2(&socket, REP_PRINT_ABORT, 0, NULL);
	return REPLY_OK;
}

//--- _do_save_file_hdr ----------------------------------------------------------
static int _do_save_file_hdr(RX_SOCKET socket, SFSDirEntry *msg)
{
	SReply		reply;
	char		path[MAX_PATH];

	TrPrintfL(1, "_do_save_file_hdr >>%s<<", msg->name);

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_SAVE_FILE_HDR;

	memcpy(&_DirEntry, msg, sizeof(_DirEntry));

	sprintf(path, "%s%s", PATH_FONTS, msg->name);
	rx_mkdir(PATH_FONTS);
	_File = rx_fopen(path, "wb", _SH_DENYNO);
	
	reply.reply=REPLY_OK;

	sok_send(&socket, &reply);

	return reply.reply;
}

//--- _do_save_file_block -------------------------------------------------
static int _do_save_file_block	(RX_SOCKET socket, SDataBlockMsg *msg)
{
	int 	len, written;
	SReply	reply;

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_SAVE_FILE_BLOCK;
	reply.reply		 = REPLY_OK;

	if (_File)
	{
		len = msg->hdr.msgLen - sizeof(msg->hdr);
		if (len)
		{
			written = (int)fwrite(msg->data, 1, len, _File);
			if (written==len) reply.reply=REPLY_OK;
			else			  reply.reply=REPLY_ERROR;
		}
		else // close the file
		{
			fclose(_File);
			_File = NULL;
//			chmod(_DirEntry.name, S_IRWXU | S_IRWXG | S_IRWXO);
		}
	}

	sok_send(&socket, &reply);

	return reply.reply;
}