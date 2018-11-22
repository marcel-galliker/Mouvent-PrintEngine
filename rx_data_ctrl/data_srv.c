// ****************************************************************************
//
//	data_srv.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_sok.h"
#include "tcp_ip.h"

#include "data_srv.h"

//--- Defines -----------------------------------------------------------------
#define MAX_CONNECTIONS 16

typedef struct
{
	RX_SOCKET	socket;
	char		addr[64];
	char		filename[128];
	FILE		*file;
	UINT64		filesize;
	UINT64		pos;
	int			time0;
	int			time;
} SConnection;

//--- Externals ---------------------------------------------------------------


//--- Modlue Globals -----------------------------------------------------------------
HANDLE  sDataSrv=NULL;

SConnection _Connection[MAX_CONNECTIONS]={0};


//--- Prototypes ------------------------------------------------------
int _handle_data_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
int _handle_data_client_connected(RX_SOCKET socket, const char *peerName);
int _handle_data_client_closed(RX_SOCKET socket, const char *peerName);

SConnection* _connection(RX_SOCKET socket);

void _handle_open	(RX_SOCKET socket, SDsOpenCmd *pmsg);
void _handle_read	(RX_SOCKET socket, SDsReadCmd *pmsg);
void _handle_close	(RX_SOCKET socket, SDsCloseCmd *pmsg);


//--- data_init ------------------------------------------
void data_init(void)
{
	if (sDataSrv == NULL)
	{
		sok_start_server(&sDataSrv, NULL, PORT_DATA_SERVER, SOCK_STREAM, MAX_CONNECTIONS, _handle_data_msg, _handle_data_client_connected, _handle_data_client_closed);
	}
}

//--- _connection --------------------------------------
SConnection* _connection(RX_SOCKET socket)
{
	int i;
	for (i=0; i<SIZEOF(_Connection); i++)
	{
		if (_Connection[i].socket==socket) return &_Connection[i];
		if (_Connection[i].socket==0)      return &_Connection[i];
	}
	return NULL;
}

//--- _handle_data_client_connected ------------------------------------------------
int _handle_data_client_connected(RX_SOCKET socket, const char *peerName)
{
	SConnection* pconnection = _connection(socket);

	if (pconnection)
	{
		pconnection->socket = socket;
		strcpy(pconnection->addr, peerName);
		data_update_status();
	}
	return REPLY_OK;
}

//--- _handle_data_client_closed --------------------------------------------------
int _handle_data_client_closed(RX_SOCKET socket, const char *peerName)
{
	SConnection* pconnection = _connection(socket);
	if (pconnection) 
	{
		memset(pconnection, 0, sizeof(*pconnection));
		data_update_status();
	}
	return REPLY_OK;
}

//--- _handle_data_msg ----------------------------------------------------------
int _handle_data_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr *phdr = (SMsgHdr*) msg;
//	TrPrintfL(FALSE, "Data:  Received cmd=0x%08x", phdr->msgId);
	switch(phdr->msgId)
	{
	case CMD_DS_OPEN:	_handle_open(socket, (SDsOpenCmd*)msg);		break;
	case CMD_DS_READ:	_handle_read(socket, (SDsReadCmd*)msg);		break;
	case CMD_DS_CLOSE:	_handle_close(socket, (SDsCloseCmd*)msg);	break;
	default: Error(WARN, 0, "Received unnown msgId 0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//--- _handle_open -------------------------------------------------
void _handle_open	(RX_SOCKET socket, SDsOpenCmd *pmsg)
{
	SDsOpenRep msg;
	SConnection* pconnection = _connection(socket);

	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId  = REP_DS_OPEN;
	msg.file_id    = 1;
	msg.filetime   = rx_file_get_mtime(pmsg->filepath);
	msg.size	   = pconnection->filesize = rx_file_get_size(pmsg->filepath);
	pconnection->pos=0;
	strcpy(pconnection->filename, pmsg->filepath);
	pconnection->file = rx_fopen(pmsg->filepath, "rb", _SH_DENYNO);
	sok_send(&socket, &msg);
}

//--- _handle_read ----------------------------------------------------
void _handle_read	(RX_SOCKET socket, SDsReadCmd *pmsg)
{
	UINT i;
	int size;
	SDsReadRep *msg = malloc(sizeof(SDsReadRep)-1+pmsg->block_size);
	SConnection* pconnection = _connection(socket);

	if (pconnection->time0==0) pconnection->time0 = rx_get_ticks();
	for (i=0; i<pmsg->cnt; i++)
	{
	//	rx_file_seek(_file, pmsg->block*pmsg->block_size, SEEK_SET);
		pconnection->pos= pmsg->block*pmsg->block_size;
		msg->hdr.msgId  = REP_DS_READ;
		msg->file_id    = pmsg->file_id;
		msg->block		= pmsg->block+i;
		size			= (int)fread(msg->data, 1, pmsg->block_size, pconnection->file);
		msg->hdr.msgLen = sizeof(SDsReadRep)-1+ size;
		sok_send(&socket, msg);
	}
	pconnection->time = rx_get_ticks();
	free(msg);
}

//--- _handle_close -----------------------------------------------------
void _handle_close	(RX_SOCKET socket, SDsCloseCmd *pmsg)
{
	SConnection* pconnection = _connection(socket);
	if (pconnection && pconnection->file)
	{
		pconnection->pos=pconnection->filesize;
		fclose(pconnection->file);
	}
}

//--- data_update_status ----------------------------------------------------
void data_update_status(void)
{
	int i, t;

	goto_xy(0,7);
	printf(" # | Sok  | Addr                 | File                                                          | %%  | ms     | MB/s\n");
	printf("---|------|----------------------|---------------------------------------------------------------|----|--------|-----\n");

	for (i=0; i<SIZEOF(_Connection); i++)
	{
		t = _Connection[i].time-_Connection[i].time0;
		if (!t) t=1;
		if (_Connection[i].filesize)
			printf("%2d: %5d  %-22s %-64s %-3d  %3d'%03d   %-3d\n", i+1, _Connection[i].socket, _Connection[i].addr, _Connection[i].filename, (int)(100*_Connection[i].pos/_Connection[i].filesize), t/1000, t%1000, (int)(1000.0*_Connection[i].pos/t/1024/1024));
		else
			printf("%2d: %5d  %-22s %-64s                   \n", i+1, _Connection[i].socket, _Connection[i].addr, _Connection[i].filename);
	}
}