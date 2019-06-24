// ****************************************************************************
//
//	rip_clnt.c	Ergosoft Rip Interface
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************
//
//
//
// Documentation for HTML messages: http://www.html-world.de/209/request-methoden/
//

// #include <io.h>

#ifndef linux
	#include <winsock2.h>
	#include <iphlpapi.h>
#endif

#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "tcp_ip.h"
#include "gui_svr.h"
#include "bmp.h"
#include "es_rip.h"
#include "rip_clnt.h"

//--- Defines -----------------------------------------------------------------
//#define ESRIP_PORT	8080

//#define ESRIP_PATH_PRINTENV	"Data\\PrEnvFiles\\"
// #define MAX_RIP_JOBS	10
//--- Externals ---------------------------------------------------------------



//--- Modlue Globals -----------------------------------------------------------------
static int			_RipThreadRunning;
static RX_SOCKET	_RipSocket = INVALID_SOCKET;
static RX_SOCKET	_GuiSocket;
static HANDLE		_Mutex=NULL;
static HANDLE		_Sem=NULL;
static SRipEvt		_RipEvt;

#define DIR_ENTRIES	10


static SFSDirEntry	_DirEntry[DIR_ENTRIES];
static int			_DirEntryIn;
static int			_DirEntryOut;
static FILE*		_File = NULL;
static char			_LocalPath[MAX_PATH];
static UINT64		_FileSize;
static UINT64		_Time;
static UINT32		_FileId;
static UINT32		_Block;
static UINT32		_BlockCheck;
static UINT32		_BlockCnt;
static int			_BlockSize = (8*1024); // 1248*4;
static UINT32		_BlockBurst = 100;


static char _path_user[MAX_PATH];
static char _path_exe [MAX_PATH];
static char _path_jobQueue[MAX_PATH];


//--- Prototypes --------------------------------------------------------------
static int  _connect			(void);
static void _rip_done			(const char *jobName);
static int  _handle_rip_msg		(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
static int	_connection_closed	(RX_SOCKET socket, const char *peerName);
static void _do_rip_evt			(SRipEvt *pmsg);

static void _handle_dir_entry(RX_SOCKET socket, SFSDirEntry *pmsg);
static int  _handle_open (RX_SOCKET socket, SDsOpenRep *pmsg);
static int  _handle_read (RX_SOCKET socket, SDsReadRep *pmsg);
static int  _handle_close(RX_SOCKET socket, SDsCloseRep *pmsg);
static void _copy_next(RX_SOCKET socket);
static void _read_next	 (RX_SOCKET socket);
static void _close_transfer(RX_SOCKET socket, UINT32 id);

static void *_rip_thread(void *ppar);

//--- rip_start -------------------------------------------------------
void rip_start(void)
{
	_RipThreadRunning = TRUE;
	if (_Mutex==NULL) _Mutex=rx_mutex_create();
	if (_Sem==NULL)   _Sem=rx_sem_create();
	rx_thread_start(_rip_thread, NULL, 0, "Rip Thread");
}

//--- _rip_thread ---------------------------------------------------------
static void *_rip_thread(void *ppar)
{
	while (_RipThreadRunning)
	{
		rx_sleep(1000);
		if (_connect()==REPLY_OK)
		{
			rip_send_print_env(INVALID_SOCKET);
			rip_set_config(INVALID_SOCKET);
		}
	}
	return NULL;
}

//--- _connect ------------------------------------------
static int _connect(void)
{
	if (RX_Config.ergosoft.ripIpAddr[0]==0) return REPLY_ERROR;
	if (_RipSocket==0 || _RipSocket==INVALID_SOCKET) 
	{
		UINT32 speed;
		memset(_DirEntry, 0, sizeof(_DirEntry));
		_DirEntryIn = _DirEntryOut = 0;
		if (sok_open_client_2(&_RipSocket, RX_Config.ergosoft.ripIpAddr, PORT_RIP_SERVER, SOCK_STREAM, _handle_rip_msg, _connection_closed)==REPLY_OK)
		{
			#ifdef linux
				speed = 1000000; // sok_get_netspeed("em2");
			#else
				speed = sok_get_netspeed(_RipSocket);
			#endif
			if (speed && speed<1000000) Error(WARN, 0, "Network to RIP slow: only %d MBaud", speed/1000000);
			return REPLY_OK;
		}
	}
	return REPLY_ERROR;
}

//--- rip_send_print_env -----------------------------
void rip_send_print_env	(RX_SOCKET socket)
{
	if (_RipSocket!=INVALID_SOCKET)
	{
		rx_mutex_lock(_Mutex);
		_GuiSocket = socket;
		sok_send_2(&_RipSocket, INADDR_ANY, CMD_GET_PRINT_ENV, 0, NULL);
		rx_sem_wait(_Sem, 100);
		rx_mutex_unlock(_Mutex);
	}
}

//--- rip_set_config ---------------------------------------------------------------
void	rip_set_config(RX_SOCKET socket)
{
	if (_RipSocket != INVALID_SOCKET)	{
		//--- send bitmap split config (all info) --------------------------
		int color;
		for (color = 0; color < SIZEOF(RX_Color); color++)
		{
			RX_Color[color].no = color;
			sok_send_2(&_RipSocket, INADDR_ANY, CMD_COLOR_CFG, sizeof(RX_Color[color]), &RX_Color[color]);
		}
	}
}

static int _handle_rip_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr* phdr = (SMsgHdr*)pmsg;

	switch(phdr->msgId)
	{
	case REP_GET_PRINT_ENV:	TrPrintfL(TRUE, "REP_GET_PRINT_ENV"); gui_send_msg(_GuiSocket, pmsg); break;
	case BEG_GET_PRINT_ENV: TrPrintfL(TRUE, "BEG_GET_PRINT_ENV"); gui_send_msg(_GuiSocket, pmsg); break;
	case ITM_GET_PRINT_ENV: TrPrintfL(TRUE, "ITM_GET_PRINT_ENV >>%s<<", ((SPrintEnvEvt*)pmsg)->printEnv); gui_send_msg(_GuiSocket, pmsg); break;
	case END_GET_PRINT_ENV: TrPrintfL(TRUE, "END_GET_PRINT_ENV"); gui_send_msg(_GuiSocket, pmsg); 
							rx_sem_post(_Sem);
							break;

	case REP_RIP_START:		TrPrintfL(TRUE, "REP_RIP_START");
							rx_sem_post(_Sem);
							break;
						
	case EVT_RIP_START:		TrPrintfL(TRUE, "EVT_RIP_START");
							_do_rip_evt((SRipEvt*)pmsg);
							break;

	case EVT_RFS_DIR_ENTRY:	_handle_dir_entry(socket, (SFSDirEntry*) pmsg); break;
	case REP_DS_OPEN:		_handle_open (socket, (SDsOpenRep*) pmsg); break;
	case REP_DS_READ:		_handle_read (socket, (SDsReadRep*) pmsg); break;
	case REP_DS_CLOSE:		_handle_close(socket, (SDsCloseRep*)pmsg); break;

	default: 		Error(WARN, 0, "Unknown Command 0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}


//--- _connection_closed ----------------------------------------
static int	_connection_closed	(RX_SOCKET socket, const char *peerName)
{
	TrPrintf(TRUE, "Connection >>%s<< closed", peerName);
	sok_close(&_RipSocket);
	return REPLY_OK;
}

//--- _do_rip_evt -------------------------------------------------------
static void _do_rip_evt(SRipEvt *pmsg)
{
	TrPrintfL(TRUE, "rip[%d]: >>%s<< state >>%s<<", pmsg->jobId, pmsg->jobName, pmsg->state);
	memcpy(&_RipEvt, pmsg, sizeof(_RipEvt));
	if (!*_RipEvt.state)
	{
		_RipEvt.jobId=0;
		TrPrintfL(TRUE, "RIP DONE");
	} 
}

//--- rip_file_ripped ----------------------------------------------------------
int  rip_file_ripped(const char *jobName, const char *printEnv, char *dataPath, char *ripState)
{
	char filename[100];
	char ext[10];

	*ripState = 0;

	split_path(jobName, NULL, filename, ext);

	int ret = esrip_file_ripped(jobName, printEnv, dataPath);
	TrPrintfL(TRUE, "esrip_file_ripped=%d", ret);
	if (ret!=REPLY_OK || !strcmp(_RipEvt.jobName, filename))
	{
		if (_RipEvt.jobId)
		{
			strcpy(ripState, _RipEvt.state);
			if( !*_RipEvt.state) return REPLY_OK;
			else return REPLY_NOT_RIPPED;
		}
	}
	return ret;
}

//--- rip_file -----------------------------------------------------------------
int  rip_file(const char *jobName, const char *filePath, const char *printEnv, int orientation, int columns, int colDist)
{
	SRipCmd cmd;
	char ext[10];

	if (_RipSocket == INVALID_SOCKET) return Error(ERR_CONT, 0, "Could not connect to Rip Server at >>%s<<", RX_Config.ergosoft.ripIpAddr);

	cmd.hdr.msgId  = CMD_RIP_START;
	cmd.hdr.msgLen = sizeof(cmd);
	if (jobName==NULL) split_path(filePath, NULL, cmd.jobName, ext);
	else               strncpy(cmd.jobName, jobName, sizeof(cmd.jobName));
	strncpy(cmd.filePath, filePath, sizeof(cmd.filePath));
	strncpy(cmd.printEnv, printEnv, sizeof(cmd.printEnv));
	strncpy(cmd.sourceIpAddr, RX_Config.ergosoft.sourceIpAddr, sizeof(cmd.sourceIpAddr));
	cmd.orientation = orientation;
	cmd.columns = columns;
	cmd.colDist = colDist;

	sok_send(&_RipSocket, &cmd);
	return REPLY_OK;
}


//--- _handle_dir_entry -----------------------------------
static void _handle_dir_entry(RX_SOCKET socket, SFSDirEntry *pmsg)
{
	int idx1;

	TrPrintfL(TRUE, "_handle_dir_entry >>%s<<", pmsg->name);
	idx1 = (_DirEntryIn+1) % DIR_ENTRIES; 

	if (idx1 == _DirEntryOut)
	{
		Error(WARN, 0, "Directory is full");
		return;
	}
	else
	{
		memcpy(&_DirEntry[_DirEntryIn], pmsg, sizeof(_DirEntry[_DirEntryIn]));
		_DirEntryIn = idx1;
		_copy_next(socket);
	}
}

//--- _copy_next --------------------------------------------------
static void _copy_next(RX_SOCKET socket)
{
	if (_File==NULL && _DirEntryOut!=_DirEntryIn)
	{
		SDsOpenCmd  msg;
		char		dir[MAX_PATH];

		TrPrintfL(TRUE, "Copy File >>%s<<", _DirEntry[_DirEntryOut].name);
		strcpy(_LocalPath, _DirEntry[_DirEntryOut].name);
		split_path(_LocalPath, dir, NULL, NULL);
		rx_mkdir(dir);
		_File = rx_fopen(_LocalPath, "wb", _SH_DENYNO);
		msg.hdr.msgId  = CMD_DS_OPEN;
		msg.hdr.msgLen = sizeof(msg);
		strncpy(msg.filepath, _LocalPath, sizeof(msg.filepath));
		sok_send(&socket, &msg);
		_DirEntryOut = (_DirEntryOut+1) % DIR_ENTRIES;
	}
}

//--- _handle_open ----------------------------------------------------------------------------
static int  _handle_open (RX_SOCKET socket, SDsOpenRep *pmsg)
{
	UINT64 time, size;

	strcpy(_LocalPath, pmsg->filepath);
	time = rx_file_get_mtime(_LocalPath);
	size = _FileSize = pmsg->size;
//	if (time<pmsg->filetime)
	{
		TrPrintfL(TRUE, "TRANSFER START >>%s<<, id=%d, size=%lld, time=%lld", _LocalPath, pmsg->file_id, pmsg->size, pmsg->filetime);

		_Time		= rx_get_ticks();
		_FileId		= pmsg->file_id;
		_Block		= 0;
		_BlockCheck = 0;
		_BlockCnt	= (int)((pmsg->size+_BlockSize-1) / _BlockSize);
		_read_next(socket);
	}
//	else _close_transfer(socket, pmsg->file_id);

	return REPLY_OK;
}

//--- _handle_read -----------------------------------------------------------------------------
static int  _handle_read (RX_SOCKET socket, SDsReadRep *pmsg)
{
	int size = pmsg->hdr.msgLen - (sizeof(SDsReadRep)-1);

	if (_File)
	{
		if (pmsg->block != _BlockCheck) TrPrintf(TRUE, "ERROR: Expected block %d, received block %d", _BlockCheck, pmsg->block); 
		if (!(pmsg->block%100000)) TrPrintfL(TRUE, "received block %d / %d: size=%d", pmsg->block, _BlockCnt, size);
		fwrite(pmsg->data, 1, size, _File);
		_BlockCheck++;
		if (_BlockCheck>=_Block) _read_next(socket);
	}
	return REPLY_OK;
}

//--- _handle_close ----------------------------------------------------------------------------
static int  _handle_close(RX_SOCKET socket, SDsCloseRep *pmsg)
{
	return REPLY_OK;
}

//--- _read_next ------------------------------------------
static void _read_next(RX_SOCKET socket)
{
	if (_Block<_BlockCnt && _File)
	{
		SDsReadCmd	msg;
		msg.hdr.msgId  = CMD_DS_READ;
		msg.hdr.msgLen = sizeof(msg);
		msg.file_id	   = _FileId;
		msg.block      = _Block;
		msg.block_size = _BlockSize;
		msg.cnt		   = _BlockCnt-_Block;
		if (msg.cnt>_BlockBurst) msg.cnt=_BlockBurst;
		_Block += msg.cnt;
		sok_send(&socket, &msg);
	}
	else _close_transfer(socket, _FileId);
}


//--- _close_transfer -----------------------------------------------
static void _close_transfer(RX_SOCKET socket, UINT32 id)
{
	int time = rx_get_ticks()-(int)_Time;
	if (time==0) TrPrintfL(TRUE, "Transfer[%d] >>%s<< ended, time=%d", id, _LocalPath, time);
	else		 TrPrintfL(TRUE, "Transfer[%d] >>%s<< ended, time=%d, speed=%dMB/Sec", id, _LocalPath, time,  _FileSize*1000/1024/1024/time);
	if (_File)
	{
		fclose(_File);
		_File = NULL;
		_copy_next(socket);
	}
}