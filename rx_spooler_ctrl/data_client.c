// ****************************************************************************
//
//	data_client.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "ctrl_client.h"
#include "data_client.h"
#include <errno.h>

//--- defines -----------------------------------------------------------------
#define MNT_PATH	"/mnt/ripped-data"

//--- globals -----------------------------------------------------------------
static int			_IsLocal;
static int			_IsMain;
static int			_ThreadRunning;
static RX_SOCKET	_DataSok;
static char			_FilePath[MAX_PATH];
static char			_LocalPath[MAX_PATH];
static FILE			*_File;
static int			_Mounted=FALSE;
static int			_Abort;

static HANDLE		_Transmitted=NULL;
static int			_SignalTransmitted=FALSE;
static int			_Time;
static int			_FileId;
static INT64		_FileSize;
static int			_BlockSize = (8*1024); // 1248*4;
static int			_Block;
static int			_BlockCheck;
static int			_BlockCnt;
static UINT32		_BlockBurst = 100;

//--- prototyes ---------------------------------------------------------------
static void  _clear_cache(void);
static void *_dc_thread(void *par);
static int  _dc_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
static void _read_next(void);
static void _close_socket(RX_SOCKET socket, int id);
static int  _handle_open (RX_SOCKET socket, SDsOpenRep *pmsg);
static int  _handle_read (RX_SOCKET socket, SDsReadRep *pmsg);
static int  _handle_close(RX_SOCKET socket, SDsCloseRep *pmsg);

//--- dc_start ----------------------------------------------------------------
int dc_start(RX_SOCKET socket, const char *ipAddr_dataServer)
{
	char name[100], addr[32];
	int port;
	static RX_SOCKET _socket=INVALID_SOCKET;

	if (socket==_socket) return REPLY_OK;
	_socket=socket;
	sok_get_socket_name(socket, name, addr, &port);
	_IsMain = !strcmp(addr, RX_CTRL_MAIN);
	if (!_stricmp(ipAddr_dataServer, "localhost") || _IsMain || !strcmp(addr, "127.0.0.1"))
	{
		_IsLocal	   = TRUE;
		_ThreadRunning = FALSE;
	}
	else
	{
		_IsLocal	   = FALSE;
		_ThreadRunning = TRUE;
		_Transmitted = rx_sem_create();
		_SignalTransmitted = FALSE;
		rx_thread_start(_dc_thread, (void*)ipAddr_dataServer, 0, "data_client");
	}
	return REPLY_OK;
}

//--- dc_end ------------------------------------------------------------------
int dc_end(void)
{
	if (!_IsLocal)
	{
		_ThreadRunning = FALSE;
		rx_sem_destroy(&_Transmitted);
	}
	return REPLY_OK;
}

//--- dc_is_local -------------------------------------
int dc_is_local(void)
{
	return _IsLocal;
}

//--- _dc_thread ---------------------------------------------------------------
static void *_dc_thread(void *par)
{
	const char *addr = (const char*) par;
	int			ret;

	while (_ThreadRunning)
	{
		ret = sok_open_client(&_DataSok, addr, PORT_DATA_SERVER, SOCK_STREAM);
		if (ret==REPLY_OK)
		{
		//	dc_read_File("D:/PrintData/Radex 4C/HolidayBeef_none/file0.usf");
		//	dc_read_File("D:\\Ergosoft\\Radex 4C\\test5-Label\\test5-Label.usf");

			TrPrintfL(TRUE, "DataClient: Connected to Server");
			ret = sok_receiver(NULL, _DataSok, _dc_handle_msg, NULL);
			TrPrintfL(TRUE, "DataClient: Detached from Server");
		}
		rx_sleep(1000);
	}
	return NULL;
}

//--- _dc_handle_msg ------------------------------------------------------------------------
static int  _dc_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*) msg;

	switch(phdr->msgId)
	{
	case REP_DS_OPEN:	_handle_open (socket, (SDsOpenRep*) msg); break;
	case REP_DS_READ:	_handle_read (socket, (SDsReadRep*) msg); break;
	case REP_DS_CLOSE:	_handle_close(socket, (SDsCloseRep*)msg); break;
	}
	return REPLY_OK;
}

//--- dc_local_path ------------------------------------------------------------------------
int dc_local_path(const char *filepath, char *localPath)
{
	char name[MAX_PATH];
	int len;
	char *ch;

	if (_IsLocal)
	{
		strcpy(localPath, filepath);
	}
	else if (_IsMain)
	{
		//--- create local path --------------
		strcpy(_FilePath, filepath);
		split_path(_FilePath, NULL, name, NULL);
		

		//--- convert to linux path ---
		if (filepath[1]==':')
		{
			len = sprintf(localPath, "%s", PATH_RIPPED_DATA);
			strcpy(&localPath[len], &filepath[3]);
			for (ch=localPath; *ch; ch++)
				if (*ch=='\\') *ch='/';
		}			
	}
	else
	{
		if (!_Mounted) _Mounted = rx_file_mount("//" RX_CTRL_MAIN "/ripped-data", MNT_PATH, "radex", "radex")==REPLY_OK;
		strcpy(localPath, filepath);
	}

	return REPLY_OK;
}

//--- dc_cache_file ------------------------------------------------------------------------
int dc_cache_file(SPageId *pid, char *colorSN, const char *filePath, char *localPath)
{
	int ret=0;
	char mntPath[MAX_PATH];
	char cmd[MAX_PATH];
	time_t filetime, localtime;

	_Abort = FALSE; 
	dc_local_path(filePath, localPath);

	if (_IsLocal) return REPLY_OK;

	sprintf(mntPath, "%s%s", MNT_PATH, &localPath[strlen(PATH_RIPPED_DATA)]);

	filetime  = rx_file_get_mtime(mntPath);
	localtime = rx_file_get_mtime(localPath);

	if (localtime==filetime)  return REPLY_OK;

	TrPrintfL(TRUE, "copy file %s", localPath);

	//--- create directory -----
	split_path(localPath, cmd, NULL, NULL);
	rx_mkdir_path(cmd);
	{
		FILE *src=NULL;
		FILE *dst=NULL;
		BYTE *buf=NULL;
		int bufsize=10240;
		int len;
		int time, last=0;
		INT64 size;
		INT64 copied=0;

		src = fopen(mntPath, "rb");
		dst = fopen(localPath, "wb");
		buf = malloc(bufsize);
		if (src && dst && buf)
		{
			size = rx_file_get_size(mntPath);
			while ((len=fread(buf, 1, bufsize, src)) && !_Abort)
			{
				if (fwrite(buf, 1, len, dst)!=len) break;
				copied+=len;
				time = rx_get_ticks()/1000;
				if (time!=last) 
				{
					last=time;
					ctrl_send_load_progress(pid, colorSN, (int)(100*copied/size));
				}
			}
		}
		TrPrintfL(TRUE, "end Transfer: errno=%d\n", errno);
		if(buf) free(buf);
		if (src) fclose(src);
		if (dst) fclose(dst);
		if (_Abort) remove(localPath);
		if (errno==ENOSPC) 
		{
			_clear_cache(); // disk full!
			return REPLY_ERROR;
		}
		rx_file_set_mtime(localPath, filetime);
	}
	ctrl_send_load_progress(pid, colorSN, 100);
	TrPrintfL(TRUE, "File cached, ret=%d", ret);
	return ret;		
}

//--- _clear_cache -------------------------------------------
static void  _clear_cache(void)
{	
	char cmd[MAX_PATH];

	sprintf(cmd, "rm -rf %s", PATH_RIPPED_DATA);
	int ret = system(cmd);
	Error(ERR_ABORT, 0, "Cach was full. Is cleared now!");
}

//--- dc_abort -----------------------------------------------
int dc_abort(void)
{
	TrPrintfL(TRUE, "dc_abort");
	_Abort = TRUE;
	if (_File)
	{
		_close_socket(_DataSok, _FileId);
	}
	return REPLY_OK;
}

//--- dc_read_file ---------------------------------------------------------------------------
int dc_read_file(const char *filepath, const char *localPath)
{
	SDsOpenCmd  msg;
	
	if (!_DataSok) return Error(ERR_STOP, 0, "Not Connected to Data Server");

	//--- create local path --------------
	strcpy(_FilePath, filepath);
	strcpy(_LocalPath, localPath);

	//--- open file ----------------------------
	msg.hdr.msgId  = CMD_DS_OPEN;
	msg.hdr.msgLen = sizeof(msg);
	strcpy(msg.filepath, filepath);
	sok_send(&_DataSok, &msg);
	_SignalTransmitted=TRUE;
	if (rx_sem_wait(_Transmitted, 100*1000)) return Error(ERR_STOP, 0, "Data Client: Timeout");
	return REPLY_OK;
}

//--- _handle_open ----------------------------------------------------------------------------
int  _handle_open (RX_SOCKET socket, SDsOpenRep *pmsg)
{
	UINT64 time, size;

	time = rx_file_get_mtime(_LocalPath);
	size = _FileSize = rx_file_get_size(_LocalPath);
	if (time<pmsg->filetime || size<pmsg->size)
	{
		TrPrintfL(TRUE, "TRANSFER START >>%s<<, id=%d, size=%d, time=%d", _LocalPath, pmsg->file_id, pmsg->size, pmsg->filetime);

		_Time		= rx_get_ticks();
		_File		= rx_fopen(_LocalPath, "wb", _SH_DENYNO);
		_FileId		= pmsg->file_id;
		_Block		= 0;
		_BlockCheck = 0;
		_BlockCnt	= (int)((pmsg->size+_BlockSize-1) / _BlockSize);
		_read_next();
	}
	else _close_socket(socket, pmsg->file_id);

	return REPLY_OK;
}

//--- _handle_read -----------------------------------------------------------------------------
int  _handle_read (RX_SOCKET socket, SDsReadRep *pmsg)
{
	int size = pmsg->hdr.msgLen - (sizeof(SDsReadRep)-1);

	if (_File)
	{
		if (pmsg->block != _BlockCheck) TrPrintf(TRUE, "ERROR: Expected block %d, received block %d", _BlockCheck, pmsg->block); 
		if (!(pmsg->block%100000)) TrPrintfL(TRUE, "received block %d / %d: size=%d", pmsg->block, _BlockCnt, size);
		fwrite(pmsg->data, 1, size, _File);
		_BlockCheck++;
		if (_BlockCheck>=_Block) _read_next();
	}
	return REPLY_OK;
}

//--- _handle_close ----------------------------------------------------------------------------
int  _handle_close(RX_SOCKET socket, SDsCloseRep *pmsg)
{
	return REPLY_OK;
}

//--- _read_next ------------------------------------------
void _read_next(void)
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
		sok_send(&_DataSok, &msg);
	}
	else _close_socket(_DataSok, _FileId);
}

//--- _close_socket ----------------------------------------------
static void _close_socket(RX_SOCKET socket, int id)
{
	SDsCloseCmd msg;
	UINT64 size, gb, mb, b;

	if (_File) 
	{
		int t=rx_get_ticks()-_Time;
		fclose(_File);
		size = rx_file_get_size(_LocalPath);
		b  = size%1000; size/=1000;
		mb = size%1000;
		gb = size/1000;
		if (_Block>=_BlockCnt) TrPrintfL(TRUE, "TRANSFER DONE >>%s<< time=%d ms, size=%d'%03d'%03d, speed=%d MB/s", _LocalPath, t, gb, mb, b, (int)(size/t)	);
		else 
		{
			TrPrintfL(TRUE, "TRANSFER Aboorted");
			remove(_LocalPath);
		}
	}
	_File=NULL;

	msg.hdr.msgId  = CMD_DS_CLOSE;
	msg.hdr.msgLen = sizeof(msg);
	msg.file_id    = id;
	sok_send(&socket, &msg);
	if (_SignalTransmitted) rx_sem_post(_Transmitted);
	_SignalTransmitted = FALSE;
}
