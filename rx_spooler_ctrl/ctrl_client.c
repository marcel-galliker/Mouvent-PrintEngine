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
#include "data.h"
#include "data_client.h"
#include "bmp.h"
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
static SFSDirEntry		_DirEntry;
static FILE				*_File=NULL;
static SPageId			_LastPageId;
static BYTE*			_Buffer[MAX_COLORS]; // [MAX_HEADS_COLOR];	// buffered in case of same image

int					_MsgGot, _MsgSent, _MsgGot0;
int					_MsgId=0;

//--- prototypes --------------------------------------------------------------

static void *_main_ctrl_thread(void *par);
static int _handle_main_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);

static int _do_spool_cfg	(RX_SOCKET socket, SSpoolerCfg	  *cfg);
static int _do_color_cfg	(RX_SOCKET socket, SColorSplitCfg *cfg);
static int _do_print_file	(RX_SOCKET socket, SPrintFileCmd  *msg);
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
	memset(_Buffer, 0, sizeof(_Buffer));

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

//--- ctrl_send_load_progress ---------------------------------------------------------
void ctrl_send_load_progress(SPageId *id, const char *colorSN, int progress)
{
	SPrintFileMsg msg;

	TrPrintfL(TRUE, "Loading >>%s<<, %d%%", colorSN, progress);

	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId  = EVT_PRINT_FILE;
	msg.evt        = DATA_LOADING;
	msg.spoolerNo  = RX_SpoolerNo;
	msg.bufReady   = data_ready();
	memcpy(&msg.id, id, sizeof(SPageId));
	snprintf(msg.txt, sizeof(msg.txt), "%s %d%%", colorSN, progress);
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
			rx_exit(0);
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

	_MsgId = phdr->msgId;
	switch(phdr->msgId)
	{
	case CMD_PING:					break;

	case CMD_SET_SPOOL_CFG:			_do_spool_cfg		(socket, (SSpoolerCfg*)		&phdr[1]);	break;
	case CMD_HEAD_BOARD_CFG:		hc_head_board_cfg	(socket, (SHeadBoardCfg*)	&phdr[1]);	break;
	case CMD_COLOR_CFG:				_do_color_cfg		(socket, (SColorSplitCfg*)	&phdr[1]);	break;
	case CMD_PRINT_FILE:			_MsgGot0++;
									_PrintFile_Socket = socket;
									memcpy(&_PrintFile_Msg, msg, sizeof(_PrintFile_Msg));
									rx_sem_post(_PrintFile_Sem);
									break;

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

	default:					Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
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
	data_init(socket, RX_Spooler.colorCnt*RX_Spooler.headsPerColor);
	_Running = FALSE;
	_Abort   = FALSE;
	_MsgGot = _MsgSent = _MsgGot0 = 0;
	memset(&_LastPageId, 0, sizeof(_LastPageId));
	sok_send_2(&socket, INADDR_ANY, REP_SET_SPOOL_CFG, 0, NULL);
	sr_reset();
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
			if (RX_Spooler.printerType==printer_DP803)	RX_ColorNameInit(cfg->inkSupplyNo, cfg->rectoVerso, cfg->color.colorCode);
			else										RX_ColorNameInit(cfg->inkSupplyNo, rv_undef, cfg->color.colorCode);				
		}
	}
	else Error(ERR_CONT, 0, "CMD_COLOR_CFG no=%d out of range", cfg->no);
	return REPLY_OK;
}

//--- _do_print_file ----------------------------------------------------------------------
static int _do_print_file(RX_SOCKET socket, SPrintFileCmd  *pmsg)
{
	SPrintFileCmd   msg;
	SPrintFileMsg	evt;
	int				ret=REPLY_OK;
	int				same;
	static SPrintFileRep	reply;
	static char				path[MAX_PATH];

	_MsgGot++;

	memcpy(&msg, pmsg, sizeof(msg)); // local copy as original can be overwritten by the communication task!

	_Running = TRUE;
	hc_start_printing();

	same = (msg.id.id == _LastPageId.id) && (msg.id.page==_LastPageId.page);

	TrPrintfL(TRUE, "_do_print_file >>%s<< id=%d, page=%d, copy=%d, same=%d", msg.filename, msg.id.id, msg.id.page, msg.id.copy, same);

	if (!same)
	{
	//	sr_mnt_path(msg.filename, path);
		data_cache(&msg.id, msg.filename, path, RX_Color, SIZEOF(RX_Color));

		//--- allocate memory ---------------------------------------------------------------------
		if ( msg.variable)// && !*path)
		{
			sr_get_label_size(&reply.widthPx, &reply.lengthPx, &reply.bitsPerPixel);
		}
		else
		{
			ret = data_get_size(path, msg.id.page, msg.gapPx, &reply.widthPx, &reply.lengthPx, &reply.bitsPerPixel);
			if (ret==REPLY_NOT_FOUND) 
				Error(ERR_STOP, 0, "Could not load file >>%s<<", msg.filename);
		}
		if (ret==REPLY_OK)	    ret = data_malloc (msg.printMode, reply.widthPx, reply.lengthPx, reply.bitsPerPixel, RX_Color, SIZEOF(RX_Color), _Buffer);
		if (_Abort) 
			return REPLY_ERROR;
		if (ret==REPLY_ERROR)
			return Error(ERR_STOP, 0,     "Could not allocate memory file >>%s<<, page=%d, copy=%d, scan=%d", path, msg.id.page, msg.id.copy, msg.id.scan);
//		is handled in main!
//		if (msg.lengthUnit==PQ_LENGTH_COPIES)
//			msg.lengthPx = msg.lengthPx*reply.widthPx; // scanning --> width!
	}

	//--- send feedback ---------------------------------------------------------------------
	reply.hdr.msgId  = REP_PRINT_FILE;
	reply.hdr.msgLen = sizeof(reply);
	if (!msg.clearBlockUsed) reply.lengthPx=0;
	memcpy(&reply.id, &msg.id, sizeof(reply.id));
	reply.bufReady   = data_ready();
	ret=sok_send(&socket, &reply);

	if (ret==0) _MsgSent++;
	else
		TrPrintfL(TRUE, "sok_send PRINT-REPLY=%d", ret);

	TrPrintfL(TRUE, "_MsgGot0=%d, _MsgGot=%d, _MsgSent=%d", _MsgGot0, _MsgGot, _MsgSent);
//	if (_PrintFileDone_Sem) 	rx_sem_post(_PrintFileDone_Sem);
	 
	if (ret==REPLY_OK || !*path)
	{
		if (data_load(&msg.id, path, msg.offsetWidth, msg.lengthPx, msg.gapPx, msg.blkNo, msg.printMode, msg.variable, msg.mirror, msg.clearBlockUsed, same, _Buffer)!=REPLY_OK)
			return Error(ERR_STOP, 0, "Could not load file >>%s<<", str_start_cut(msg.filename, PATH_RIPPED_DATA));			
		
		//--- send spool state -------------------------------------------------------------
 		evt.hdr.msgId   = EVT_PRINT_FILE;
		evt.hdr.msgLen  = sizeof(evt);
		memcpy(&evt.id, &msg.id, sizeof(evt.id));
		evt.spoolerNo   = RX_SpoolerNo;
		evt.evt			= DATA_SENDING;
		evt.bufReady    = reply.bufReady;
		sok_send(&socket, &evt);
		
		TrPrintfL(TRUE, "REPLY EVT_PRINT_FILE, bufReady=%d", evt.bufReady);
		hc_send_next();
		memcpy(&_LastPageId, &msg.id, sizeof(_LastPageId));
	}
	return REPLY_OK;
}

//--- _do_print_abort ----------------------------------------------------------------------
static int _do_print_abort(RX_SOCKET socket)
{
	TrPrintfL(TRUE, "_do_print_abort");
	_Abort=TRUE;
	data_abort();
	hc_abort_printing();
	sok_send_2(&socket, INADDR_ANY, REP_PRINT_ABORT, 0, NULL);
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