// ****************************************************************************
//
//	DIGITAL PRINTING - (Remote)file_server.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------------
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "errno.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "file_server.h"

//--- module globals -----------------------------------------------------------------

HANDLE 		_HFileServer=NULL;

SFSDirEntry	_DirEntry;
FILE		*_File=NULL;

//--- prototypes ---------------------------------------------------------------------
static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);

static int _do_make_dir			(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_del_dir 			(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_get_dir 			(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_kill_process		(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_start_process	(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_del_file			(RX_SOCKET socket, SFSDirCmd *msg);
static int _do_save_file_hdr	(RX_SOCKET socket, SFSDirEntry *msg);
static int _do_save_file_block	(RX_SOCKET socket, SDataBlockMsg *msg);

static int _client_connected		(RX_SOCKET socket, const char *peerName);
static int _client_disconncted	(RX_SOCKET socket, const char *peerName);

static UINT32 _Port;

//--- fs_start ---------------------------------------
void fs_start(const char *deviceName)
{
	char addr[32];

	if (_HFileServer) 
	{
		TrPrintfL(1, "*** fs_start: Already running ***");
		return;
	}

	sok_get_ip_address_str(deviceName, addr, sizeof(addr));
	if (sok_start_server(&_HFileServer, addr, INADDR_ANY, SOCK_STREAM, 5, _handle_msg, _client_connected, _client_disconncted)==REPLY_OK)
	{
		sok_get_server_name(_HFileServer, NULL, NULL, &_Port);
		TrPrintfL(1, "*** fs_start: started at port=%d ***", _Port);
	}
	else
	{
		char err[100];
		err_system_error(errno, err, sizeof(err));
		Error(ERR_CONT, 0, "Error >>%s<<", err);
		return;
	}	
}

//--- fs_end ----------------------------------------------------------------
void fs_end(void)
{
	static int _ending=FALSE;
	if (!_ending)
	{
		_ending= TRUE;
		if (_HFileServer) sok_stop_server(&_HFileServer);
		_ending=FALSE;
	}
	else TrPrintfL(1, "stop\n");	
	if (_File) fclose(_File);
	_File = NULL;
}

//--- fs_get_port ---------------------------------------
int  fs_get_port(void)
{
	if (_HFileServer)	return _Port;
	else				return 0;
}

//--- _client_connected ----------------------------------------------------
static int _client_connected(RX_SOCKET socket, const char *peerName)
{
	TrPrintfL(TRUE, "RFS Client Connected >>%s<<", peerName);
	return REPLY_OK;
}

//--- _client_disconncted ---------------------------------------------------
static int _client_disconncted(RX_SOCKET socket, const char *peerName)
{			
	TrPrintfL(TRUE, "RFS Client Disconnected >>%s<<", peerName);
	fs_end();
	return REPLY_OK;
}

//--- _handle_msg -----------------------------------------------------------
static int _handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr	*hdr = (SMsgHdr*)msg;

	switch (hdr->msgId)
	{
	case CMD_RFS_MAKE_DIR:			_do_make_dir			(socket, (SFSDirCmd*)msg); break;
	case CMD_RFS_DEL_DIR:			_do_del_dir 			(socket, (SFSDirCmd*)msg); break;
	case CMD_RFS_GET_DIR:			_do_get_dir 			(socket, (SFSDirCmd*)msg); break;
	case CMD_RFS_DEL_FILE:			_do_del_file			(socket, (SFSDirCmd*)msg); break;
	case CMD_RFS_SAVE_FILE_HDR:		_do_save_file_hdr	(socket, (SFSDirEntry*)msg); break;
	case CMD_RFS_SAVE_FILE_BLOCK:	_do_save_file_block	(socket, (SDataBlockMsg*)msg); break;
	case CMD_RFS_KILL_PROCESS:		_do_kill_process		(socket, (SFSDirCmd*)msg); break;
	case CMD_RFS_START_PROCESS:		_do_start_process	(socket, (SFSDirCmd*)msg); break;
	}
	return REPLY_OK;
}

//--- _do_make_dir --------------------------------------------------------------
static int _do_make_dir(RX_SOCKET socket, SFSDirCmd* msg)
{
	// creates the complete directory tree

	char *ch,  ch1;
	struct stat info;
	SReply		reply;

	TrPrintfL(1, "mkdir >>%s<<", msg->name);
	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_MAKE_DIR;
	reply.reply=REPLY_OK;
	for (ch = msg->name; *ch && reply.reply==REPLY_OK; ch++)
	{
		if (*ch=='/')
		{
			ch1 = ch[1];
			ch[1]=0;
			if (stat(msg->name, &info)) 
				reply.reply=rx_mkdir(msg->name);
			ch[1]=ch1;
		}
		else if (*ch==0)
		{
			reply.reply = rx_mkdir(msg->name);
		}
	}

	sok_send(&socket, &reply);
	return reply.reply;
}

//--- _do_del_dir --------------------------------------------------------------
static int _do_del_dir (RX_SOCKET socket, SFSDirCmd* msg)
{
	SReply		reply;

	TrPrintfL(1, "rmdir >>%s<<", msg->name);
	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_DEL_DIR;

	reply.reply = rx_rmdir(msg->name);

	return reply.reply;
}

//--- _do_get_dir --------------------------------------------------------------
static int _do_get_dir (RX_SOCKET socket, SFSDirCmd* msg)
{
	SEARCH_Handle	search;
	SFSDirEntry		entry;

	TrPrintfL(1, "dir >>%s<<", msg->name);
	search = rx_search_open(msg->name, "*");
	if (search)
	{
//		entry.hdr.msgLen = sizeof(entry);
//		entry.hdr.msgId	 = REP_RFS_GET_DIR;
//
		while (rx_search_next(search, entry.name, sizeof(entry.name), &entry.timeModified, &entry.size, &entry.flags))
		{
			if (entry.name[0]!='.')
			{
				entry.hdr.msgLen = sizeof(entry);
				entry.hdr.msgId	 = EVT_RFS_DIR_ENTRY;
				sok_send(&socket, &entry);
			}
		}
		memset(&entry, 0, sizeof(entry));
		entry.hdr.msgLen = sizeof(entry);
		entry.hdr.msgId	 = EVT_RFS_DIR_ENTRY;
		sok_send(&socket, &entry);

		rx_search_close(search);
	}
	return REPLY_OK;
}

//--- _do_kill_process --------------------------------------------------------------
static int _do_kill_process (RX_SOCKET socket, SFSDirCmd* msg)
{
	SReply		reply;

	TrPrintfL(1, "_do_kill_process >>%s<<", msg->name);

	rx_process_kill(msg->name, NULL);

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_KILL_PROCESS;
	reply.reply		 = REPLY_OK;

	return reply.reply;
}

//--- _do_start_process --------------------------------------------------------------
static int _do_start_process (RX_SOCKET socket, SFSDirCmd* msg)
{
	SReply		reply;
	int			cnt_proc, cnt_gdb;

	TrPrintfL(1, "_do_start_process >>%s<<", msg->name);

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_START_PROCESS;
	reply.reply		 = REPLY_OK;

	cnt_proc = rx_process_running_cnt(msg->name, NULL);
	cnt_gdb  = rx_process_running_cnt("gdbserver", msg->name);
	TrPrintfL(1, "Count >>%s<<: proc=%d, gdb=%d", msg->name, cnt_proc,  cnt_gdb);
	if (!cnt_proc && !cnt_gdb) 
	{
		reply.reply = rx_process_start(msg->name, NULL);
		TrPrintfL(1, "PROCESS STARTED");
	}
	else TrPrintfL(1, "process >>%s<< is already running", msg->name);

	return reply.reply;
}

//--- _do_del_file --------------------------------------------------------------
static int _do_del_file(RX_SOCKET socket, SFSDirCmd* msg)
{
	SReply		reply;

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_DEL_FILE;

	reply.reply = remove(msg->name);

	sok_send(&socket, &reply);
	return reply.reply;
}

//--- _do_save_file_hdr ----------------------------------------------------------
static int _do_save_file_hdr(RX_SOCKET socket, SFSDirEntry *msg)
{
	SReply		reply;
	int			tries;

	TrPrintfL(1, "_do_save_file_hdr >>%s<<", msg->name);

	rx_process_kill(msg->name, NULL);

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_SAVE_FILE_HDR;

	memcpy(&_DirEntry, msg, sizeof(_DirEntry));

	#ifdef linux
		mode_t process_mask = umask(0);
	#endif
	for (tries=0; _File==NULL && tries<5; tries++)
	{
		_File = fopen(msg->name, "w");
		if (errno==ETXTBSY) rx_sleep(100);
	}
	#ifdef linux
		umask(process_mask);
	#endif

	if (_File==NULL) 	
	{
		char err[100];
		err_system_error(errno, err, sizeof(err));
		Error(ERR_CONT, 0, "fopen(>>%s<<) Error %d >>%s<<", msg->name, errno, err);
		
		reply.reply = errno;
	}
	else reply.reply=REPLY_OK;

	sok_send(&socket, &reply);

	return reply.reply;
}

//--- _do_save_file_block --------------------------------------------------------
static int _do_save_file_block	(RX_SOCKET socket, SDataBlockMsg *msg)
{
	int 	len, written;
	SReply	reply;

//	printf("_do_save_file_block ");

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId  = REP_RFS_SAVE_FILE_BLOCK;

	if (_File)
	{
		len = msg->hdr.msgLen - sizeof(msg->hdr);
//		printf("len=%d\n", len);
		if (len)
		{
			written = fwrite(msg->data, 1, len, _File);
			if (written==len) reply.reply=REPLY_OK;
			else			  reply.reply=REPLY_ERROR;
		}
		else // close the file
		{
			TrPrintfL(1, "file saved");

			fclose(_File);
			_File = NULL;
			
			#ifdef linux
			chmod(_DirEntry.name, S_IRWXU | S_IRWXG | S_IRWXO);
			#endif

			rx_file_set_mtime(_DirEntry.name, _DirEntry.timeModified);
			reply.reply = REPLY_OK;
		}
	}
	else reply.reply=REPLY_ERROR;

	sok_send(&socket, &reply);

	return reply.reply;
}
