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
#include "screening.h"
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


//--- load file ---------------------------------
static void				*_load_file_thread(void *par);
#define LOAD_FILE_BUF_SIZE	8
static SLoadFileCmd	_LoadFileBuf[LOAD_FILE_BUF_SIZE];
static int			_LoadFileInIdx;
static int			_LoadFileOutIdx;
HANDLE				_LoadFile_Sem;

static void				*_print_file_thread(void *par);
static HANDLE			_PrintFile_Sem;
static RX_SOCKET		_PrintFile_Socket;
static SPrintFileCmd	_PrintFile_Msg;

static int				_Running;
static int				_Abort;
static int				_data_malloc_reply;
static int				_ResetCnt;
static int				_StartCnt;
static int				_FirstFile;
static int				_Paused;
static SFSDirEntry		_DirEntry;
static FILE				*_File=NULL;
static char				_LastFilename[MAX_PATH];
static int				_LastPage;
static int				_LastWakeup;
static int				_LastGap;
static int				_LastOffsetWidth;
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
static int _do_density		(RX_SOCKET socket, SDensityMsg *pmsg);
static int _do_load_file	(RX_SOCKET socket, SLoadFileCmd  *msg);

static int _do_print_file	(RX_SOCKET socket, SPrintFileCmd  *msg);
static void _do_start_sending(UINT32 resetCnt);
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
	_LastOffsetWidth = 0;
	_LastWakeup = 0;
	_LastGap = 0;
	_SMP_Flags=0;
	_LoadFileInIdx  = 0;
	_LoadFileOutIdx = 0;

	rx_mem_init(512*1024*1024);
	if (!_PrintFile_Sem) _PrintFile_Sem = rx_sem_create();

	rx_thread_start(_main_ctrl_thread, (void*)ipAddrMain, 0, "_main_ctrl_thread");
	rx_thread_start(_load_file_thread,  NULL,             0, "_load_file_thread");
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

//--- ctrl_get_bufferNo ---------------------------------------------------------
int ctrl_get_bufferNo(BYTE *ptr)
{
	int no, n;
	if (ptr!=NULL)
	{
		for (no=0; no<SIZEOF(_Buffer); no++)
		{
			for (n=0; n<SIZEOF(_Buffer[no]); n++)
			{
				if (ptr>=_Buffer[no][n] && ptr<_Buffer[no][n]+_BufferSize[no]) return 100*no+n;				
			}
		}		
	}
	return -1;
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
static int _handle_main_ctrl_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr	*sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;
	void    *pdata = &phdr[1];
	static UINT32 _UnknownMsgId=0;

	_MsgId = phdr->msgId;
	switch(phdr->msgId)
	{
	case CMD_PING:					break;

	case CMD_SET_SPOOL_CFG:			_do_spool_cfg		(socket, (SSpoolerCfg*)		pdata);	break;
	case CMD_HEAD_BOARD_CFG:		hc_head_board_cfg	(socket, (SHeadBoardCfg*)	pdata);	break;
	case CMD_COLOR_CFG:				_do_color_cfg		(socket, (SColorSplitCfg*)	pdata);	break;
	case CMD_SET_DENSITY:			_do_density			(socket, (SDensityMsg*)		pmsg);	break;
    case CMD_LOAD_FILE:				_do_load_file		(socket, (SLoadFileCmd*)	pmsg);	break;
	case CMD_PRINT_FILE:			_MsgGot0++;
									_PrintFile_Socket = socket;
									memcpy(&_PrintFile_Msg, pmsg, sizeof(_PrintFile_Msg));
									rx_sem_post(_PrintFile_Sem);
									break;

	case CMD_START_PRINTING:		_do_start_sending	(*(UINT32*)pdata);						break;
	case CMD_PRINT_ABORT:			_do_print_abort		(socket);								break;
	
	case BEG_SET_LAYOUT:			sr_set_layout_start (socket, (char*)pdata);					break;
	case ITM_SET_LAYOUT:			sr_set_layout_blk   (socket, phdr);							break;
	case END_SET_LAYOUT:			sr_set_layout_end   (socket, phdr);							break;
	
	case BEG_SET_FILEDEF:			sr_set_filedef_start(socket, phdr);							break;
	case ITM_SET_FILEDEF:			sr_set_filedef_blk  (socket, phdr);							break;
	case END_SET_FILEDEF:			sr_set_filedef_end  (socket, phdr);							break;

	case CMD_SET_CTRDEF:			ctr_set_def	((SCounterDef*)		pdata);						break;
		
	case CMD_PRINT_DATA:			sr_data_record		((SPrintDataMsg*) pmsg);				break;

	case CMD_PRINT_TEST_DATA:		_do_print_test_data(socket, (SPrintTestDataMsg*)pmsg);	    break;

	case CMD_RFS_SAVE_FILE_HDR:		_do_save_file_hdr	(socket, (SFSDirEntry*)  pmsg);			break;
	case CMD_RFS_SAVE_FILE_BLOCK:	_do_save_file_block(socket, (SDataBlockMsg*)pmsg);			break;
//	case CMD_FONTS_UPDATED:			rip_add_fonts(PATH_FONTS);									break;
	case CMD_FONTS_UPDATED:			rip_add_fonts((char*)pdata);								break;

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
	_data_malloc_reply = REPLY_OK;
	_ResetCnt	 = 0;
	_FirstFile	 = TRUE;
	_Paused		 = FALSE;
	_StartCnt	 = RX_Spooler.resetCnt;
	if (hc_in_simu()) _ResetCnt = _StartCnt; 
	_SMP_Flags	 = 0;
	_MsgGot = _MsgSent = _MsgGot0 = 0;
	if (!rx_def_is_tx(RX_Spooler.printerType)) data_send_id(NULL);
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

//--- _do_density ---------------------------------------------------------------------
static int _do_density(RX_SOCKET socket, SDensityMsg *pmsg)
{
	jc_set_disabled_jets(pmsg);
	scr_set_values(pmsg->head, pmsg->data.densityValue);
	return REPLY_OK;
}

//--- _do_load_file ---------------------------------------------------
static int _do_load_file(RX_SOCKET socket, SLoadFileCmd  *pdata)
{
	int idx=(_LoadFileInIdx+1) % LOAD_FILE_BUF_SIZE;
	if (idx==_LoadFileOutIdx) return Error(ERR_ABORT, 0, "Loadfile buffer overflow");
	memcpy(&_LoadFileBuf[_LoadFileInIdx], pdata, sizeof(_LoadFileBuf[_LoadFileInIdx]));
	_LoadFileInIdx=idx;
	rx_sem_post(_LoadFile_Sem);
	return REPLY_OK;
}

//---  _load_file_thread ------------------------------------------------------------
static void *_load_file_thread(void *par)
{
	_LoadFile_Sem=rx_sem_create();
	while (_ThreadRunning)
	{
		if (rx_sem_wait(_LoadFile_Sem, 1000)==REPLY_OK)
		{
			data_load_file(_LoadFileBuf[_LoadFileOutIdx].filepath, &_LoadFileBuf[_LoadFileOutIdx].id);
			_LoadFileOutIdx = (_LoadFileOutIdx+1) % LOAD_FILE_BUF_SIZE;
		}				
	}
	return NULL;
}

//--- _do_print_file ----------------------------------------------------------------------
static int _do_print_file(RX_SOCKET socket, SPrintFileCmd  *pdata)
{
	SPrintFileCmd			msg;
	int						ret=REPLY_OK;
	int						same;
	static SPrintFileRep	reply;
	static EFileType		fileType=ft_undef;
	static int				widthPx, lengthPx;
	static UCHAR			bitsPerPixel;
	static UINT8			multiCopy;
	static char				path[MAX_PATH];
	
	if (_Abort) return Error(LOG, 0, "ABORT");
	if (_data_malloc_reply) return REPLY_ERROR;
	
	_MsgGot++;
	memcpy(&msg, pdata, sizeof(msg)); // local copy as original can be overwritten by the communication task!

	_Running = TRUE;
	hc_start_printing();
		
	same = (!strcmp(msg.filename, _LastFilename) &&  msg.id.page==_LastPage && msg.wakeup==_LastWakeup && msg.gapPx==_LastGap);
//	if (rx_def_is_lb(RX_Spooler.printerType)) same &= msg.offsetWidth==_LastOffsetWidth;
	if (rx_def_is_lb(RX_Spooler.printerType) && msg.printMode==PM_SINGLE_PASS) same = ((msg.flags&FLAG_SAME)!=0) && (msg.offsetWidth==_LastOffsetWidth);
	if (msg.printMode==PM_SINGLE_PASS && jc_changed()) same=FALSE;
	_LastPage   = msg.id.page;
	_LastGap	= msg.gapPx;
	_LastWakeup = msg.wakeup;
	_LastOffsetWidth = msg.offsetWidth;
	
//	Error(LOG, 0, "_do_print_file (id=%d, page=%d, copy=%d) FLAG_SAME=%d, same=%d", msg.id.id, msg.id.page, msg.id.copy, msg.flags&FLAG_SAME, same);

	if (msg.virtualPass>msg.virtualPasses)
		Error(ERR_ABORT, 0, "programming Error");
	
	TrPrintfL(TRUE, "_do_print_file[%d] >>%s<<  id=%d, page=%d, copy=%d, scan=%d, same=%d, clearBlockUsed=%d, offsetWidth=%d, blkNo=%d", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, same, msg.clearBlockUsed, msg.offsetWidth, msg.blkNo);
//	if (msg.id.scan==1) Error(LOG, 0, "_do_print_file[%d] >>%s<<  id=%d, page=%d, copy=%d, scan=%d, same=%d, blkNo=%d", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan ,same, msg.blkNo);
	
//	if(msg.smp_flags & SMP_LAST_PAGE) Error(LOG, 0, "_do_print_file[%d] >>%s<< id=%d, page=%d, copy=%d, scan=%d, same=%d, LAST PAGE", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, same); 
//	else                                Error(LOG, 0, "_do_print_file[%d] >>%s<< id=%d, page=%d, copy=%d, scan=%d, same=%d", _MsgGot, msg.filename, msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, same); 
	
//	Error(LOG, 0, "Spooler Copy=%d, Offset=%d", msg.id.copy, msg.offsetWidth);
	
	if (!same)
	{
	//	Error(LOG, 0, "load file >>%s<< id=%d, page=%d, copy=%d", msg.filename, msg.id.id, msg.id.page, msg.id.copy);

		if(msg.printMode==PM_SCANNING && rx_def_is_tx(RX_Spooler.printerType))
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
			data_get_size(path, msg.id.page, &fileType, &msg.gapPx, &widthPx, &lengthPx, &bitsPerPixel, &multiCopy);
			TrPrintfL(TRUE, "data_get_size >>%s<<: gapPx=%d, widthPx=%d, lengthPx=%d, bitsPerPixel=%d, multiCopy=%d", path, msg.gapPx, widthPx, lengthPx, bitsPerPixel, multiCopy);
			if (ret==REPLY_NOT_FOUND) 
			{
				if (_MsgGot==1) Error(ERR_ABORT, 0, "Could not load file >>%s<<", msg.filename);
				else Error(ERR_STOP, 0, "Could not load file >>%s<<", msg.filename);
			}
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
		reply.offsetWidth = msg.offsetWidth;
		reply.clearBlockUsed = msg.clearBlockUsed;
		reply.bufReady   = data_ready();
		if (sok_send(&socket, &reply)==REPLY_OK) _MsgSent++;
	}

	if (!same)
	{
		if (msg.printMode==PM_SCAN_MULTI_PAGE) multiCopy=1;
		if (fileType!=ft_undef)
		{
			_BufferNo = (_BufferNo+1)%BUFFER_CNT;
			if (msg.smp_bufSize) 
			{
				Error(LOG, 0, "data_clear");				
				data_clear(_Buffer[_BufferNo]);
			}
			TrPrintfL(TRUE, "data_malloc (id=%d, page=%d, copy=%d, scan=%d) _BufferNo=%d", msg.id.id, msg.id.page, msg.id.copy, msg.id.scan, _BufferNo);
			_data_malloc_reply = data_malloc (msg.printMode, widthPx, lengthPx, bitsPerPixel, RX_Color, SIZEOF(RX_Color), &_BufferSize[_BufferNo], _Buffer[_BufferNo]);
		}
		if (_Abort)
			return REPLY_ERROR;
		if (_data_malloc_reply!=REPLY_OK)
		{
			Error(ERR_STOP, 0,  "Could not allocate memory file >>%s<<, page=%d", path, msg.id.page);
			if (multiCopy>1)	Error(LOG, 0,   "Make Image width a multilpe of 4 (multicopy=%d)", multiCopy);
			return REPLY_ERROR;
		}
	}

	TrPrintfL(TRUE, "REP_PRINT_FILE _MsgGot0=%d, _MsgGot=%d, _MsgSent=%d, widthPx=%d, lengthPx=%d, offset=%d, same=%d", _MsgGot0, _MsgGot, _MsgSent, widthPx, lengthPx, msg.offsetWidth, same);
//	if (_PrintFileDone_Sem) 	rx_sem_post(_PrintFileDone_Sem);
	 
	if (ret==REPLY_OK || !*path)
	{ 
		SPrintFileMsg	evt;
		if (msg.flags & FLAG_SMP_LAST_PAGE) _SMP_Flags |= FLAG_SMP_LAST_PAGE;
		
		if (msg.flags&FLAG_SAME)
		{
			data_same(&msg.id, msg.offsetWidth, msg.clearBlockUsed);				
		}
		else
		{
			if (data_load(&msg.id, path, fileType, msg.offsetWidth, msg.lengthPx, multiCopy, msg.gapPx, msg.blkNo, reply.blkCnt, msg.printMode, msg.variable, msg.virtualPasses , msg.virtualPass, msg.flags, msg.clearBlockUsed, same, msg.smp_bufSize, msg.dots, _Buffer[_BufferNo])!=REPLY_OK && !_Abort)
				return Error(ERR_STOP, 0, "Could not load file >>%s<<", str_start_cut(msg.filename, PATH_RIPPED_DATA));
		}

		if (_Abort) return REPLY_OK;

		//--- send spool state -------------------------------------------------------------
 		evt.hdr.msgId   = EVT_PRINT_FILE;
		evt.hdr.msgLen  = sizeof(evt);
		memcpy(&evt.id, &msg.id, sizeof(evt.id));
		evt.spoolerNo   = RX_SpoolerNo;
		evt.evt			= DATA_SENDING;
		evt.bufReady    = reply.bufReady;
		sok_send(&socket, &evt);
		
		TrPrintfL(TRUE, "REPLY EVT_PRINT_FILE, bufReady=%d, _ResetCnt=%d, ResetCnt=%d", evt.bufReady, _ResetCnt, RX_Spooler.resetCnt);
		if(rx_def_is_tx(RX_Spooler.printerType) && (_FirstFile || !same))
		{
			if (_FirstFile) 
            {
                TrPrintfL(TRUE, "_do_print_file: FirstFile: data_send_id(NULL)");
                data_send_id(NULL);
            }
			if (arg_tracePQ) Error(LOG, 0, "Data Loaded: id=%d _ResetCnt=%d, _StartCnt=%d, RX_Spooler.resetCnt=%d", msg.id.id, _ResetCnt, _StartCnt, RX_Spooler.resetCnt);				
			if (_ResetCnt==_StartCnt)
			{
				if (data_next_id()) _StartCnt++;
			}
			_FirstFile = FALSE;
		}
		
		if (_ResetCnt==RX_Spooler.resetCnt || hc_in_simu()) 
			hc_send_next();			
			
		memcpy(&_LastFilename, &msg.filename, sizeof(_LastFilename));
	}
	return REPLY_OK;
}

//--- _do_start_sending ----------------------------
static void _do_start_sending(UINT32 resetCnt)
{
	if (arg_tracePQ) Error(LOG, 0, "_do_start_sending(%d), RX_Spooler.resetCnt=%d", resetCnt, RX_Spooler.resetCnt);
	if (RX_Spooler.resetCnt && resetCnt>RX_Spooler.resetCnt) RX_Spooler.resetCnt=resetCnt;
	if (resetCnt==RX_Spooler.resetCnt) 
	{
		_ResetCnt = _StartCnt = resetCnt;
		if(rx_def_is_tx(RX_Spooler.printerType) && data_next_id()) _StartCnt++;
		hc_send_next();
	}
}
	
//--- _do_print_abort ----------------------------------------------------------------------
static int _do_print_abort(RX_SOCKET socket)
{
	TrPrintfL(TRUE, "_do_print_abort");
	_Abort=TRUE;
	data_abort();
	hc_abort_printing();
	sok_send_2(&socket, REP_PRINT_ABORT, 0, NULL);

	//--- memory management (large files on TX) -----------------------
	{
		// always start with _Buffer[0], free all other buffers!
		_BufferNo = BUFFER_CNT-1;
		for(int b=1; b<BUFFER_CNT; b++)
		{
			data_free(&_BufferSize[b], _Buffer[b]);
		}
	}
	
	//--- test prints: force reloading file at each print start ---------
	if (str_start(_LastFilename, PATH_BIN_SPOOLER)) 
		memset(&_LastFilename, 0, sizeof(_LastFilename));

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