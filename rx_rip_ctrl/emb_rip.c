// ****************************************************************************
//
// emb_rip.c	Embedded Rip Interface
//
// ****************************************************************************
//
//	Copyright 2017 by BM-Printing SA, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************
//
//
//
// Documentation for HTML messages: http://www.html-world.de/209/request-methoden/
//

// #include <io.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mem.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "tcp_ip.h"
#include "bmp.h"
#include "rx_tif.h"
#include "rx_embrip.h"
#include "emb_rip.h"

//--- Defines -----------------------------------------------------------------
#define MAX_RIP_JOBS	10

//--- Externals ---------------------------------------------------------------



//--- Module Globals -----------------------------------------------------------------
typedef struct
{
	char		name[MAX_PATH];
	char		path[MAX_PATH];
	int			pos;
	RX_SOCKET	socket;
} SRipJobInfo;

static int			_ThreadRunning;
static int			_Running;
static BYTE*		_Buffer[MAX_COLORS];	// buffered in case of same image

static void			*_rip_file_thread(void *par);
static HANDLE		_RipFile_Sem;
static RX_SOCKET	_RipFile_Socket;
static SRipCmd		_RipFile_Msg;

static HANDLE		_check_thread_id;
static int			_check_thread_running;
static SRipJobInfo	_rip_job[MAX_RIP_JOBS];
static UINT64		_rip_jq_time;



//--- Prototypes --------------------------------------------------------------
static int _embprintEnv_exists(const char *printEnv);
static int _do_rip_file(SRipCmd  *pmsg);

//--- esrip_init ----------------------------------------------------------------
void embrip_init(void)
{
	_ThreadRunning = TRUE;
	_Running = FALSE;
	memset(_Buffer, 0, sizeof(_Buffer));

	rx_mem_init(512 * 1024 * 1024);
	if (!_RipFile_Sem) _RipFile_Sem = rx_sem_create();

	rx_thread_start(_rip_file_thread, NULL, 0, "_rip_file_thread");
}


//--- esrip_end ------------------------------------------------------------------
void embrip_end(void)
{
	_check_thread_running = FALSE;
}


//--- _printEnv_exists ------------------------------
static int _embprintEnv_exists(const char *printEnv)
{
	char path[MAX_PATH];
	sprintf(path, "%s%s", PATH_EMBRIP_PRENV, printEnv);

	return (rx_dir_exists(path));
}


//--- embrip_rip_file -----------------------------------------------------------------
int  embrip_rip_file(RX_SOCKET socket, const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int colDist)
{

	_RipFile_Socket = socket;

	if (!_embprintEnv_exists(printEnv)) return Error(ERR_CONT, 0, "Print Environment >>%s<< not found", printEnv);
	_RipFile_Socket = socket;
	memcpy(_RipFile_Msg.jobName,	jobName,	sizeof(_RipFile_Msg.jobName));
	memcpy(_RipFile_Msg.filePath,	filepath,	sizeof(_RipFile_Msg.filePath));
	memcpy(_RipFile_Msg.printEnv,	printEnv,	sizeof(_RipFile_Msg.printEnv));
	_RipFile_Msg.orientation	= orientation;
	_RipFile_Msg.columns		= columns;
	_RipFile_Msg.colDist		= colDist;
	rx_sem_post(_RipFile_Sem);

	return REPLY_OK;
}

//--- embrip_send_print_env -----------------------------
void embrip_send_print_env	(RX_SOCKET socket)
{
	SPrintEnvEvt	msg;
	char			path[MAX_PATH];
	SEARCH_Handle	search;
	int				isDir;

	if (socket)
	{
		TrPrintf(TRUE, "do_get_print_env");
		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = REP_GET_PRINT_ENV;
		sok_send(&socket, &msg);
	}

	{
		msg.hdr.msgId = BEG_GET_PRINT_ENV;
		sok_send(&socket, &msg);

		msg.hdr.msgLen = sizeof(msg);
		msg.hdr.msgId = ITM_GET_PRINT_ENV;

		sprintf(path, "%s", PATH_EMBRIP_PRENV);
		search = rx_search_open(path, "*.*");
		while (rx_search_next(search, msg.printEnv, sizeof(msg.printEnv), NULL, NULL, &isDir))
		{
			if (isDir)
			{
				if (strcmp(msg.printEnv, ".") && strcmp(msg.printEnv, ".."))
					sok_send(&socket, &msg);
			}
		}
		rx_search_close(search);

		msg.hdr.msgLen = sizeof(msg.hdr);
		msg.hdr.msgId = END_GET_PRINT_ENV;
		sok_send(&socket, &msg);
	}
}

//---  _rip_file_thread ------------------------------------------------------------
static void *_rip_file_thread(void *par)
{
	while (_ThreadRunning)
	{
		if (rx_sem_wait(_RipFile_Sem, 1000) == REPLY_OK)
		{
			_do_rip_file(&_RipFile_Msg);
		}
	}
	return NULL;
}


//--- _do_rip_file ----------------------------------------------------------------------
static int _do_rip_file(SRipCmd  *pmsg)
{
	SRipCmd	msg;
	int		ret = REPLY_OK;
	char filepath[MAX_PATH];
	char inpath[MAX_PATH];
	char ljobName[MAX_PATH];
	char ext[32];
	char search[MAX_PATH];
	char * chr;

	memcpy(&msg, pmsg, sizeof(msg)); // local copy as original can be overwritten by the communication task!

	TrPrintfL(TRUE, "_do_rip_file >>%s<<", msg.jobName);
	rx_mkdir(PATH_RIPPED_DATA);
	sprintf(filepath, "%s%s", PATH_RIPPED_DATA, msg.printEnv);
	rx_mkdir(filepath);
	sprintf(filepath, "%s/%s", filepath, msg.jobName);
	rx_mkdir(filepath);
	sprintf(search, "%s/*.*", filepath);
	rx_remove_old_files(search, 0); // clean directory

	chr= msg.filePath;
	while (chr) {
		if (*chr == '/') {
			chr++; break;
		}
		chr++;
	}

	if (!chr) return Error(ERR_CONT, 0, "File: <%s> start of path not found", msg.filePath);
	sprintf(inpath, "%s%s", PATH_SOURCE_DATA, chr);
	split_path(inpath, NULL, ljobName, ext);
	chr = ext;
	while (*chr)
		*chr++ = tolower(*chr);

	if (!strncmp(ext, ".tif", 4))
		ret = rx_tiff_rip(msg.printEnv, inpath, msg.orientation, msg.columns, msg.colDist, filepath, msg.jobName);
	else if (!strncmp(ext, ".pdf", 4) || !strncmp(ext, ".ps", 3))
		ret = rx_pdf_rip(msg.printEnv, inpath, msg.orientation, msg.columns, msg.colDist, filepath, msg.jobName);
	else
		ret= Error(ERR_CONT, 0, "File extension: <%s> not supported", ext);

	return ret;
}
