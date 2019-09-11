// ****************************************************************************
//
//	rfs.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include <stdio.h>
#include <time.h>

#ifdef linux
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <errno.h>
#else
	#include <io.h>
	#include <share.h>
#endif
#include <fcntl.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "gui_msg.h"
#include "network.h"
#include "rfs.h"

//--- Defines -----------------------------------------------------------------

// #define TIMEOUT		1000
#define TIMEOUT			1000

#define MAX_THREADS		128

//--- Externals ---------------------------------------------------------------

//--- statics -----------------------------------------------------------------
int sTrace=TRUE;


//--- structures --------------------------------------------------------------

typedef struct SDir
{	
	SFSDirEntry	item;
	struct SDir	*next;
} SDir;

typedef struct
{
	char ipAddr[IP_ADDR_SIZE];
	int	 port;
	char srcBinPath[MAX_PATH];
	char deviceType[16];
	char dstBinPath[MAX_PATH];
	char process[32];
	UINT64 macAddr;
} SUpdateThreadPar;

typedef struct
{
	RX_SOCKET	socket;
	UINT32		waitId;
	HANDLE		event;
	SDir		*dir;
} SClientThreadPar;

static int				_Init=FALSE;
static SUpdateThreadPar	_ThreadPar[MAX_THREADS];

//--- Globals -----------------------------------------------------------------

//--- Prototypes ------- -----------------------------------------------
static void* _rfs_update_thread(void* lpParameter);
static void* _rfs_client_thread(void* lpParameter);

static int _handle_rfs_msg	(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par);

static void _rep_mkdir		(RX_SOCKET socket, SClientThreadPar *par);
static int  _rep_get_dir	(RX_SOCKET socket, SClientThreadPar *par, SFSDirEntry *msg);
static int  _do_dir_entry	(RX_SOCKET socket, SClientThreadPar *par, SFSDirEntry *msg);
static int  _rep_file_hdr	(RX_SOCKET socket, SClientThreadPar *par, SReply *msg);
static int  _rep_file_blk	(RX_SOCKET socket, SClientThreadPar *par, SReply *msg);

static int _rfs_mkdir		(SClientThreadPar *par, const char *name);
static int _rfs_getdir		(SClientThreadPar *par, const char *name);
static int _update_dir		(SClientThreadPar *par, const char *srcPath, const char *dstPath, SDir *dir);
static int _copyfile		(SClientThreadPar *par, const char *srcPath, const char *fname, time_t modified, const char *dstPath);

//--- rfs_reset ----------------------------------------------------------------
void rfs_reset(const char *ipAddr)
{
	int i;
	for (i=0; i<SIZEOF(_ThreadPar); i++) 
	{
		if (!strcmp(ipAddr, _ThreadPar[i].ipAddr)) memset(&_ThreadPar[i], 0, sizeof(_ThreadPar[i]));
	}	
}

/*
//--- rfs_update ----------------------------------------------------------------
int	rfs_update(const char *ipAddr, const char *srcBinPath, const char *deviceType, const char *dstPath)
{
	return rfs_update_start(ipAddr, srcBinPath, deviceType, dstPath, NULL);
}
*/

//--- rfs_update_start -----------------------------------------------------------
void rfs_update_start(UINT64 macAddr, const char *ipAddr, int port, const char *srcBinPath, const char *deviceType, const char *dstBinPath, const char *process)
{
	int i;
	
	if (!_Init)
	{
		memset(_ThreadPar, 0, sizeof(_ThreadPar));
		_Init=TRUE;
	}
	
	for (i=0; i<SIZEOF(_ThreadPar); i++) 
	{
		if (!strcmp(ipAddr, _ThreadPar[i].ipAddr))
		{
			if (_ThreadPar[i].port==port)
			{
				// TrPrintfL(TRUE, "rfs_update_start(%s) port:%d already running", ipAddr, port);
				return;
			}
			TrPrintfL(TRUE, "rfs_update_start(%s) port:%d ready, isBooted=%d", ipAddr, port, net_is_booted(macAddr));
			break;
		}
	}
		
	if (strcmp(ipAddr, _ThreadPar[i].ipAddr))
	{
		for (i=0; i<SIZEOF(_ThreadPar) && *_ThreadPar[i].ipAddr; i++);
		if (i==SIZEOF(_ThreadPar)) 
		{ 
			Error(ERR_CONT, 0, "_ThreadPar Overflow"); 
			return;
		};		
	}
	
	strcpy(_ThreadPar[i].ipAddr,		ipAddr);
	strcpy(_ThreadPar[i].srcBinPath,	srcBinPath);
	strcpy(_ThreadPar[i].deviceType,	deviceType);
	strcpy(_ThreadPar[i].dstBinPath,	dstBinPath);
	strcpy(_ThreadPar[i].process,		process);
	_ThreadPar[i].macAddr = macAddr;
	_ThreadPar[i].port	= port;

	TrPrintfL(TRUE, "rfs_update_start(%s) port:%d START THREAD", ipAddr, port);
//	Error(LOG, 0, "BOOT (%s) rfs started", ipAddr);
	
	rx_thread_start(_rfs_update_thread, &_ThreadPar[i], 0, "_rfs_update_thread");
}

//--- _rfs_update_thread -----------------------------------------------------------------------
static void* _rfs_update_thread(void* lpParameter)
{
	SUpdateThreadPar	*par = (SUpdateThreadPar*)lpParameter;
	SClientThreadPar	clientPar;
	int ret, time;
	char srcPath[MAX_PATH];
	char dstPath[MAX_PATH];
	char deviceStr[32];
	
	TrPrintf(TRUE, "_rfs_update_thread >>%s:%d<<", par->ipAddr, par->port);
	
	memset(&clientPar, 0, sizeof(clientPar));
	clientPar.event = rx_sem_create();

	char_to_lower(par->deviceType, deviceStr);
	sprintf(srcPath, "%s%s/", par->srcBinPath, deviceStr);
	sprintf(dstPath, "%s%s/", par->dstBinPath, deviceStr);

	rx_sem_wait(clientPar.event, 100);
	rx_sleep(100);
//	sok_ping(par->ipAddr);
//	rx_sleep(100);
	TrPrintfL(TRUE, "RFS connecting to >>%s:%d<<", par->ipAddr, par->port);
//	Error(LOG, 0, "BOOT (%s) rfs connected", par->ipAddr);
	for (time=0; TRUE; time++)
	{
		rx_sleep(10);
		ret=sok_open_client(&clientPar.socket, par->ipAddr, par->port, SOCK_STREAM);
		if (ret == REPLY_OK) break;
		{
			if ((time%10)==9) 
					TrPrintfL(TRUE, "rfs connecting to >>%s:%d<< error=%d", par->ipAddr, par->port, ret);
 
			if (time>100)
			{
				Error(ERR_CONT, 0, "Cound not connect to >>%s:(Remote File Server)<< Error %d", par->ipAddr, ret);
				memset(lpParameter, 0, sizeof(SUpdateThreadPar));
				return NULL;
			}
		}
	}

	TrPrintf(TRUE, "rfs connected to >>%s:%d<< tries=%d", par->ipAddr, par->port, time);
	
	rx_thread_start(_rfs_client_thread, &clientPar, 0, "_rfs_client_thread");

	//--- make directory ---
	if (_rfs_mkdir (&clientPar, dstPath)) goto start;
	if (_rfs_getdir(&clientPar, dstPath)) goto start;
	if (TRUE)
	{
		SDir *d;
		char str[MAX_PATH];
		TrPrintfL(sTrace, "Remote Directory: >>%s:%d<<", par->ipAddr, par->port);
		for (d=clientPar.dir; d; d=d->next)
		{	
			sprintf(str, "    >>%s<<, modified=%s", d->item.name, ctime(&d->item.timeModified));
			str[strlen(str)-1]=0;
			TrPrintfL(sTrace, str);
		}
		TrPrintfL(sTrace, "Local directory:");
	}
	str_tolower(srcPath);
	_update_dir(&clientPar, srcPath, dstPath, clientPar.dir);
	
start:
	if (par->process && *par->process)
	{
		SFSDirCmd cmd;

		cmd.hdr.msgLen	= sizeof(cmd);
		sprintf(cmd.name, "%s%s", dstPath, par->process);

	//	#ifndef DEBUG
			cmd.hdr.msgId	= CMD_RFS_KILL_PROCESS;
			sok_send(&clientPar.socket, &cmd);
	//	#endif

		rx_sleep(100);

		cmd.hdr.msgId	= CMD_RFS_START_PROCESS;
		sok_send(&clientPar.socket, &cmd);

		TrPrintfL(sTrace, "%s: START PROCESS >>%s<<", par->ipAddr, cmd.name);
		net_booted(par->macAddr);
		rx_sleep(100);	
		Error(LOG, 0, "BOOT (%s) Started %s", par->ipAddr, cmd.name);
	}
	sok_close(&clientPar.socket);
//	rx_sem_wait(clientPar.event, TIMEOUT);
	rx_sem_destroy(&clientPar.event);

	//--- delete thread parameters -----
//	memset(lpParameter, 0, sizeof(SUpdateThreadPar));
	
	return REPLY_OK;
}

//--- _rfs_mkdir ----------------------------------------------------------------
static int _rfs_mkdir( SClientThreadPar *par, const char *name)
{
	SFSDirCmd cmd;
	cmd.hdr.msgLen	= sizeof(cmd);
	cmd.hdr.msgId	= CMD_RFS_MAKE_DIR;
	strcpy(cmd.name, name);

	par->waitId = REP_RFS_MAKE_DIR;
	sok_send(&par->socket, &cmd);
	if (rx_sem_wait(par->event, TIMEOUT)) return REPLY_ERROR;
	return REPLY_OK;
}

//--- _rfs_getdir ----------------------------------------------------------------
static int _rfs_getdir( SClientThreadPar *par, const char *name)
{
	SFSDirCmd cmd;
	cmd.hdr.msgLen	= sizeof(cmd);
	cmd.hdr.msgId	= CMD_RFS_GET_DIR;
	strcpy(cmd.name, name);

	par->waitId = REP_RFS_GET_DIR;
	sok_send(&par->socket, &cmd);
	if (rx_sem_wait(par->event, TIMEOUT)) return REPLY_ERROR;

	return REPLY_OK;
}

//--- _update_dir ------------------------------------------------------------

static int _update_dir( SClientThreadPar *par, const char *srcPath, const char *dstPath, SDir *dir)
{
	SEARCH_Handle	search;
	char			fileName[100];
	char			path[MAX_PATH];
	time_t			time;
	UINT32			filesize;
	int				isDir;
	int				copy;
	SDir			*d;

	TrPrintfL(sTrace, "_update_dir src=>>%s<< dst=>>%s<<", srcPath, dstPath);

	search = rx_search_open(srcPath, "*");
	while (rx_search_next(search, fileName, sizeof(fileName), &time, &filesize, &isDir))
	{
		if (!isDir)
		{
			sprintf(path, "local: %s%s", srcPath, fileName);
			// TrPrintfL(sTrace, "  %s time:%s\n", ffd.cFileName, ctime(&time));
			//---  search in directory ---
			for (d=dir, copy=TRUE; d; d=d->next)
			{
				if (!strcmp(fileName, d->item.name) && d->item.flags==0)
				{
					d->item.flags = 2;
					if (d->item.timeModified==time && d->item.size==filesize) copy=FALSE;
				}
			}

			if (copy) 
			{
				while(_copyfile(par, srcPath, fileName, time, dstPath));
			}
		}
	}
	rx_search_close(search);

	//--- delete unused files and free memory --------
	{
		SDir *next;
		for (d=dir, copy=TRUE; d; d=next)
		{
			if (d->item.flags==0)
			{
				SFSDirCmd msg;
				msg.hdr.msgLen = sizeof(msg);
				msg.hdr.msgId = CMD_RFS_DEL_FILE;
				sprintf(msg.name, "%s/%s", dstPath, d->item.name);
				sok_send(&par->socket, &msg);
			}
			next=d->next;
			free(d);
		}
	}
	return REPLY_OK;
}

//--- _copyfile --------------------------------------------------------------------------------
static int _copyfile( SClientThreadPar *par, const char *srcPath, const char *fname, time_t modified, const char *dstPath)
{
	SFSDirEntry		hdr;
	SDataBlockMsg	*msg;
	int				file;
	int				blkSize=2000;
	int				len, blk;
	int				size=0;
	char			ipAddr[64];

	//--- send header ---
	memset(&hdr, 0, sizeof(hdr));
	hdr.hdr.msgLen = sizeof(hdr);
	hdr.hdr.msgId  = CMD_RFS_SAVE_FILE_HDR;
	sprintf(hdr.name, "%s%s", dstPath, fname); 
	memcpy(&hdr.timeModified, &modified, sizeof(hdr.timeModified));

	sok_get_peer_name(par->socket, ipAddr, NULL, NULL);
		
	TrPrintfL(sTrace, "%s: copy file %s%s", ipAddr, dstPath, fname);

	par->waitId = REP_RFS_SAVE_FILE_HDR;
	sok_send(&par->socket, &hdr);
	if (rx_sem_wait(par->event, TIMEOUT))
	{
		Error(ERR_CONT, 0, "%s: copying file >>%s%s<<: TIMEOUT hdr", ipAddr, dstPath, fname);
		return REPLY_ERROR;
	}

	//--- open the file and send it ---------------
	sprintf(hdr.name, "%s%s", srcPath, fname);
	#ifdef WIN32
		file = sopen(hdr.name, _O_BINARY|_O_SEQUENTIAL|_O_RDONLY, _SH_DENYNO);
	#else
		file = open(hdr.name, O_RDONLY);
	#endif

	if (file)
	{
		msg = (SDataBlockMsg*)malloc(sizeof(msg)+blkSize);
		msg->hdr.msgId = CMD_RFS_SAVE_FILE_BLOCK;
	//	TrPrintfL(sTrace, "%s: copy file %s%s DO", ipAddr, dstPath, fname);
		blk = 0;
		do
		{
			//--- send blocks ---
			blk++;
			len = read(file, msg->data, blkSize);
			size+=len;
			msg->hdr.msgLen = sizeof(msg->hdr)+len;
			par->waitId = REP_RFS_SAVE_FILE_BLOCK;
			sok_send(&par->socket, msg);
			if (rx_sem_wait(par->event, TIMEOUT)) 
			{
				close(file);
				Error(ERR_CONT, 0, "%s: copying file >>%s%s<<: TIMEOUT blk %d", ipAddr, dstPath, fname, blk);
				return REPLY_ERROR;
			}
		} while (len);
		close(file);
		TrPrintfL(sTrace, "%s: copy file %s%s DONE, size=%d", ipAddr, dstPath, fname, size);
	}
	return REPLY_OK;
}

//--- _rfs_client_thread -----------------------------------------------------
static void* _rfs_client_thread(void* lpParameter)
{
	SClientThreadPar *par = (SClientThreadPar*)lpParameter;
	sok_receiver(NULL, &par->socket, _handle_rfs_msg, par);
	return REPLY_OK;
}

//--- _handle_rfs_msg -------------------------------------------------------
static int _handle_rfs_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *ppar)
{
	SMsgHdr			  *phdr = (SMsgHdr*)pmsg;
	SClientThreadPar  *par  = (SClientThreadPar*)ppar;
	switch(phdr->msgId)
	{
	case REP_RFS_MAKE_DIR:			_rep_mkdir	 (socket, par);						   break;
	case REP_RFS_GET_DIR:			_rep_get_dir (socket, par, (SFSDirEntry*)	pmsg); break;
	case EVT_RFS_DIR_ENTRY:			_do_dir_entry(socket, par, (SFSDirEntry*)	pmsg); break;
	case REP_RFS_SAVE_FILE_HDR:		_rep_file_hdr(socket, par, (SReply*)		pmsg); break;
	case REP_RFS_SAVE_FILE_BLOCK:	_rep_file_blk(socket, par, (SReply*)		pmsg); break;
	}
	return REPLY_OK;
}

//--- _rep_mkdir -------------------------------------------------------------
static void _rep_mkdir(RX_SOCKET socket, SClientThreadPar *par)
{
	if (par->waitId==REP_RFS_MAKE_DIR) rx_sem_post(par->event);
}

//--- _rep_get_dir -----------------------------------------------------------
static int _rep_get_dir(RX_SOCKET socket, SClientThreadPar *par, SFSDirEntry *msg)
{
	TrPrintfL(sTrace, "Directory: >>%s<<, modified=%s", msg->name);
	return REPLY_OK;
}

//--- _do_dir_entry ---------------------------------------------------------
static int _do_dir_entry(RX_SOCKET socket, SClientThreadPar *par, SFSDirEntry *msg)
{
#pragma warning (disable:4996)
	if (msg->name[0]==0)
	{ 
		if (par->waitId==REP_RFS_GET_DIR)
			rx_sem_post(par->event);
	}
	else
	{
		SDir	*d = (SDir*)malloc(sizeof(SDir));
		memcpy(&d->item, msg, sizeof(d->item));
		d->next = par->dir;
		par->dir=d;
		{
			time_t time=msg->timeModified;
			char str[MAX_PATH];
			sprintf(str, "    >>%s<<, modified=%s", msg->name, ctime(&time));
			str[strlen(str)-1]=0;
			TrPrintfL(sTrace, str);
		}
	}
	return REPLY_OK;
}

//--- _rep_file_hdr ------------------------------------------------------------
static int  _rep_file_hdr(RX_SOCKET socket, SClientThreadPar *par, SReply *msg)
{
	if (msg->reply) Error(ERR_CONT, 0, "REP_RFS_SAVE_FILE_HDR reply=%d", msg->reply);
	if (par->waitId==REP_RFS_SAVE_FILE_HDR) rx_sem_post(par->event);
	return msg->reply;
}

//--- _rep_file_blk ------------------------------------------------------------
static int  _rep_file_blk(RX_SOCKET socket, SClientThreadPar *par, SReply *msg)
{
//	if (msg->reply) Error(ERR_CONT, 0, "REP_RFS_SAVE_FILE_BLOCK reply=%d", msg->reply);
	if (msg->reply) return msg->reply;
	if (par->waitId==REP_RFS_SAVE_FILE_BLOCK) rx_sem_post(par->event);
	return msg->reply;
}

//--- RFS_start ---------------------------------------------------------------
/*
int RFS_start (const char *ipAddr, const char *process)
{
	RX_RX_SOCKET socket;
	SFSDirCmd cmd;

	sok_open_client(&socket, ipAddr, PORT_REMOTE_FILE_SERVER, SOCK_STREAM);
	cmd.hdr.msgLen	= sizeof(cmd);
	cmd.hdr.msgId	= CMD_FS_START_PROCESS;
	strcpy_s(cmd.name, process);
	sok_send(socket, &cmd);
	rx_sleep(100);
	sok_close(&socket);
	
	return REPLY_OK;
}
*/